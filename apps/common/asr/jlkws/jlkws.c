#include "server/audio_server.h"
#include "server/server_core.h"
#include "generic/circular_buf.h"
#include "os/os_api.h"
#include "event.h"
#include "app_config.h"
#include "jlsp_far_keyword.h"
#include "event/key_event.h"
/* #include "jlsp_kws_aec.h" */

#if (defined CONFIG_ASR_ALGORITHM) && (CONFIG_ASR_ALGORITHM == JLKWS_ALGORITHM)

const int CONFIG_KWS_RAM_USE_ENABLE = 1;

extern void aisp_resume(void);

#define ONCE_SR_POINTS	160//256

#define AISP_BUF_SIZE	(ONCE_SR_POINTS * 2)	//跑不过来时适当加大倍数
#define MIC_SR_LEN		(ONCE_SR_POINTS * 2)

#ifdef CONFIG_AEC_ENC_ENABLE

/*aec module enable bit define*/
#define AEC_EN              BIT(0)
#define NLP_EN              BIT(1)
#define ANS_EN              BIT(2)

#define AEC_MODE_ADVANCE	(AEC_EN | NLP_EN | ANS_EN)
#define AEC_MODE_REDUCE		(NLP_EN | ANS_EN)
#define AEC_MODE_SIMPLEX	(ANS_EN)

#endif

static struct {
    int pid;
    u16 sample_rate;
    u8 volatile exit_flag;
    int volatile run_flag;
    OS_SEM sem;
    s16 mic_buf[AISP_BUF_SIZE * 2];
    void *mic_enc;
    cbuffer_t mic_cbuf;
} aisp_server;

#define __this (&aisp_server)

static const float confidence[8] = {
    0.9, 0.9, 0.5, 0.5, //小杰小杰，小杰同学，播放音乐，暂停播放
    0.5, 0.5, 0.5, 0.5, //增大音量，减小音量，上一首, 下一首
};

enum {
    PLAY_MUSIC_EVENT = 4,
    STOP_MUSIC_EVENT,
    VOLUME_INC_EVENT = 7,
    VOLUME_DEC_EVENT,
    SONG_PREVIOUS_EVENT,
    SONG_NEXT_EVENT,
};

static void aisp_task(void *priv)
{
    u32 mic_len, linein_len;
    u32 time = 0, time_cnt = 0, cnt = 0;
    int ret;
    int model = 0;
    int model_size, private_heap_size, share_heap_size;
    void *kws = NULL;
    u8 *private_heap = NULL, *share_heap = NULL;
    int online = 0;

    jl_far_kws_model_get_heap_size(model, &model_size, &private_heap_size, &share_heap_size);

    private_heap = zalloc(private_heap_size);
    if (!private_heap) {
        goto __exit;
    }

    share_heap	 = zalloc(share_heap_size);
    if (!share_heap) {
        goto __exit;
    }

    kws = jl_far_kws_model_init(model, private_heap, private_heap_size, share_heap, share_heap_size, model_size, confidence, online);
    if (!kws) {
        goto __exit;
    }

    aisp_resume();

    while (1) {
        if (__this->exit_flag) {
            break;
        }

        if (!__this->run_flag) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }

        if (__this->exit_flag) {
            break;
        }

        if ((cbuf_get_data_size(&__this->mic_cbuf) < ONCE_SR_POINTS * 2)) {
            os_sem_pend(&__this->sem, 0);
            continue;
        }
        short near_data_buf[ONCE_SR_POINTS];
        mic_len = cbuf_read(&__this->mic_cbuf, near_data_buf, ONCE_SR_POINTS * 2);
        if (!mic_len) {
            continue;
        }

        time = timer_get_ms();
        ret = jl_far_kws_model_process(kws, model, (u8 *)near_data_buf, sizeof(near_data_buf));
        if (ret > 1) {
            printf("++++++++++++++++++ %d ++++++++++++++++++\n", ret);
            //add your button event according to ret
            struct key_event key = {0};

            if (ret == PLAY_MUSIC_EVENT) {
                key.action = KEY_EVENT_UP;
                key.value = KEY_LOCAL;
            } else if (ret == STOP_MUSIC_EVENT) {
                key.action = KEY_EVENT_HOLD;
                key.value = KEY_LOCAL;
            } else if (ret == VOLUME_INC_EVENT) {
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_DOWN;
            } else if (ret == VOLUME_DEC_EVENT) {
                key.action = KEY_EVENT_CLICK;
                key.value = KEY_UP;
            } else if (ret == SONG_PREVIOUS_EVENT) {
                key.action = KEY_EVENT_LONG;
                key.value = KEY_UP;
            } else if (ret == SONG_NEXT_EVENT) {
                key.action = KEY_EVENT_LONG;
                key.value = KEY_DOWN;
            }

            key.type = KEY_EVENT_USER;
            key_event_notify(KEY_EVENT_FROM_USER, &key);

            jl_far_kws_model_reset(kws);
        }

        time_cnt += timer_get_ms() - time;
        if (++cnt == 100) {
            printf("aec time :%d \n", time_cnt);
            time_cnt = cnt = 0;
        }
    }

__exit:

    if (kws) {
        jl_far_kws_model_free(kws);
    }
    if (private_heap) {
        free(private_heap);
    }
    if (share_heap) {
        free(share_heap);
    }

    __this->run_flag = 0;

}

static void enc_server_event_handler(void *priv, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_SERVER_EVENT_ERR:
    case AUDIO_SERVER_EVENT_END:
        break;
    default:
        break;
    }
}

static int aisp_vfs_fwrite(void *file, void *data, u32 len)
{
    cbuffer_t *cbuf = (cbuffer_t *)file;

    u32 wlen = cbuf_write(cbuf, data, len);
    if (wlen != len) {
        cbuf_clear(&__this->mic_cbuf);
        puts("busy!\n");
    }
    os_sem_set(&__this->sem, 0);
    os_sem_post(&__this->sem);

    return len;
}

static int aisp_vfs_fclose(void *file)
{
    return 0;
}

static const struct audio_vfs_ops aisp_vfs_ops = {
    .fwrite = aisp_vfs_fwrite,
    .fclose = aisp_vfs_fclose,
};

int aisp_open(u16 sample_rate)
{
    __this->exit_flag = 0;
    __this->mic_enc = server_open("audio_server", "enc");
    server_register_event_handler(__this->mic_enc, NULL, enc_server_event_handler);
    cbuf_init(&__this->mic_cbuf, __this->mic_buf, sizeof(__this->mic_buf));
    os_sem_create(&__this->sem, 0);
    __this->sample_rate = sample_rate;

    return thread_fork("aisp", 3, 2048, 0, &__this->pid, aisp_task, __this);
}

void aisp_suspend(void)
{
    union audio_req req = {0};

    if (!__this->run_flag) {
        return;
    }

    __this->run_flag = 0;

    req.enc.cmd = AUDIO_ENC_STOP;
    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);
    cbuf_clear(&__this->mic_cbuf);
}

void aisp_resume(void)
{
    union audio_req req = {0};

    if (__this->run_flag) {
        return;
    }
    __this->run_flag = 1;
    os_sem_set(&__this->sem, 0);
    os_sem_post(&__this->sem);

    req.enc.cmd = AUDIO_ENC_OPEN;
    req.enc.channel = 1;
    req.enc.channel_bit_map = BIT(CONFIG_AISP_MIC0_ADC_CHANNEL);
    req.enc.frame_size = ONCE_SR_POINTS * 2 * req.enc.channel;
    req.enc.volume = CONFIG_AISP_MIC_ADC_GAIN;
    req.enc.sample_rate = __this->sample_rate;
    req.enc.format = "pcm";
#if CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK0
    req.enc.sample_source = "plnk0";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_PLNK1
    req.enc.sample_source = "plnk1";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS0
    req.enc.sample_source = "iis0";
#elif CONFIG_AUDIO_ENC_SAMPLE_SOURCE == AUDIO_ENC_SAMPLE_SOURCE_IIS1
    req.enc.sample_source = "iis1";
#else
    req.enc.sample_source = "mic";
#endif
    req.enc.vfs_ops = &aisp_vfs_ops;
    req.enc.output_buf_len = req.enc.frame_size * 3;
    req.enc.file = (FILE *)&__this->mic_cbuf;
#ifdef CONFIG_AEC_ENC_ENABLE
    struct aec_s_attr aec_param = {0};
    aec_param.EnableBit = AEC_MODE_ADVANCE;
    aec_param.output_way = 0;	 //1:使用硬件回采 0:使用软件回采
    req.enc.aec_attr = &aec_param;
    req.enc.aec_enable = 0;	//默认不开
#endif

    server_request(__this->mic_enc, AUDIO_REQ_ENC, &req);
}

void aisp_close(void)
{
    if (__this->exit_flag) {
        return;
    }

    aisp_suspend();

    __this->exit_flag = 1;

    os_sem_post(&__this->sem);

    if (__this->mic_enc) {
        server_close(__this->mic_enc);
        __this->mic_enc = NULL;
    }
    thread_kill(&__this->pid, KILL_WAIT);
}

#endif

