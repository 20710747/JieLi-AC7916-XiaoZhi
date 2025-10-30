#include "robot_main.h"
#include "robot_debug.h"
#include "robot_adapter.h"
#include "robot_control.h"
#include "robot_asr.h"

#include "robot_config.h"

int asr_action_flag = 0;

int robot_asr_switch = 0;
//int puppy_asr_mode = 0;
int asr_once_flag = 0;
int asr_word_idx = 0;
int asr_wakeup_flag = 0;


extern void asr_wakeup_set(int wakeup);
extern void asr_timer_cnt_pause();
extern void asr_timer_cnt_clear(void);

void robot_asr_switch_set(uint16_t sw)
{
#if 0
    int log_flag = 0;
    if(sw<=1)//开关
    {
        log_flag = 1;
    }
    else if(sw==2)
        sw = 0;
    else if(sw==3)
    {
        sw = ABOTE_SWITCH_CMDASR;
    }
    else if(sw==4)
    {
        sw = 1;
        asr_once_flag = 1;
    }
    if(robot_asr_switch == sw)
        return ;
    if(log_flag)
    {
        Tbz_Config_Save(0,sw);
    }
    robot_asr_switch = sw;
    DBG_PRINT("robot_asr_switch_set ",robot_asr_switch);
#endif
}

int robot_asr_switch_get(int debug)
{
    #if 0
    if(debug)
    {
        DBG2_PRINT("asr_switch = ",robot_asr_switch,PUPPY_SWITCH_CMDASR);
        DBG_PRINT("asr_type = ",PUPPY_SWITCH_ASRTYPE);
    }
    #endif
    return robot_asr_switch;
}


void robot_asr_start()
{
    //Action_set2(StartTalk);
}

int robot_asr_stop()
{
    if(asr_wakeup_flag)
    {
        //Action_set2(EndTalk);
        return 1;
    }
    else
    {
        return 0;
    }
}

int robot_asr_stop2()//no play
{
    int ret = 0;
    if(asr_wakeup_flag)
        ret = 1;
    asr_wakeup_set(0);
    asr_action_flag = 0;
    return ret;
}

void robot_asr_restart()
{
    if(asr_wakeup_flag)
    {
        //Action_set(0);
        //asr_action_flag = 1;
        //Action_set2(StartTalk);
    }
}

void robot_asr_mode_switch()
{
    if(asr_wakeup_flag==0)
        robot_asr_start();
    else
        robot_asr_stop();
}

int aiasr_switch_lock = 0;
extern int robot_ai_mode;
void robot_aiasr_mode_switch()
{
    aiasr_switch_lock = 10;
    if((robot_ai_mode==1)||(robot_ai_mode==2))
    {
        robot_ai_stop2(1);
        return ;
    }
    if(robot_ai_mode==5)
    {
        robot_ai_stop2(2);
        return ;
    }
    if(asr_wakeup_flag)
    {
        robot_asr_stop();
        return ;
    }
    if(!app_music_check_wifi())
        robot_asr_start();
    else
    {
        robot_ai_start();
    }
}

int next_asr_switch = -1;

void Tbz_AsrSwitchDo()
{
    robot_main_debug_output("next_asr_switch = ",next_asr_switch);
    if(next_asr_switch!=-1)
    {
        if(next_asr_switch==1)
            os_set_play_audio("asron.mp3", NULL);
        else
            os_set_play_audio("asroff.mp3", NULL);
        robot_asr_switch_set(next_asr_switch);
    }
    next_asr_switch = -1;
}

int Tbz_AsrSwitchSet(int sw)
{
    next_asr_switch = sw;
    Tbz_AsrSwitchDo();
}
#if 0
#define PUPPY_WORD_SZ 33
//asraction
int puppy_word_action_list[PUPPY_WORD_SZ+1]=
{
    0,
    StartTalk,//1heypuppy
    EndTalk,//2 end talk
    Stand,   // 3 Stand up
    Sit_down,//4 sit down
    Get_down,//5 get down
    Turn_around,//6 turn around
    Roll_over,//7 roll over
    Good_boy,//8 Good Boy
    Go_pee_pee,//9 go pee pee
    Go_to_sleep,//10 go to sleep
    Protect_me,//11,
    Play_dead,//12 BiuBiuBiu
    Play_dead2,//13 Play_dead
    Do_ShowOne,//14 show time
    Do_stunts2,//15 do stunts
    Go_forward2,//16 move forward
    Move_back2,//17 move backward
    Left_turn_around,//18 turn left
    Right_turn_around,//19 turn right
    Turn_back,//20 Turn_back
    Go_forward3,//21 come here
    Move_back4,//22 go away
    Good_boy2,// 23 i love you
    Swing_head,//24 shake head
    Sprain_back,//25 wag tail
    Push_up,//26 do push up
    Left_leg_shaking,//27 shake left leg
    Right_leg_shaking,//28 shake right leg
    Crawling,// 29 crwaling forward
    Butt_up_and_down,//30
    Slide_step,// 31 sliding ste
    Move_back4,//32 i hate you
    No_Action,//33
};
#endif
void ASR_Action_Test(int idx)
{
    #if 0
    static int last_idx = 0;
    if(idx==-1)
    {
        idx = last_idx;
    }
    else if(idx==-2)
    {
        idx = last_idx+1;
    }
    if((idx==23)||(idx==24))//儿歌故事
    {
        idx = 25;
    }
    if((idx<=0)||(idx>=PUPPY_WORD_SZ))
    {
        idx = 1;
    }
    last_idx = idx;
    extern volatile int puppy_action_voice_flag2;
    puppy_action_voice_flag2 = 1;
    //Action_set2(puppy_word_action_list[idx]);
    #endif
}


void asr_wakeup_set(int wakeup)
{
    if(wakeup == asr_wakeup_flag)
        return ;
    switch(wakeup)
    {
    case 0:
        break;
    case 1:
        break;

    }
    asr_wakeup_flag = wakeup;
    DBG_PRINT("asr_wakeup_flag = ",asr_wakeup_flag);

}
void asr_data_make()
{
    //获取识别组
    #if 0
    int group = 1;

    if(!asr_wakeup_flag)
    {
        if(get_bt_connect_status()==47)//非唤醒状态，且蓝牙播放音乐中，就设置为0，
            group = 0;
        //后续添加播放网络音乐也为0
        extern int ai_music_flag;
        if(ai_music_flag)
            group = 0;
    }

    wanson_next_group_set(group);
    #endif
    if(asr_word_idx)
    {
        if((asr_wakeup_flag==0)&&(asr_word_idx==1))//唤醒前先切换到idle状态
        {
            //DBG_PRINT("puppy_set_idle = ",0);
            //puppy_set_idle(0,7);
            //tbz_app_set_idle();
        }
        DBG_PRINT("asr_word_idx = ",asr_word_idx);
       // puppy_action_voice_flag2 = 1;
        //Action_set2(puppy_word_action_list[asr_word_idx]);
        if((asr_word_idx==1)&&(asr_wakeup_flag==0))
        {
            asr_wakeup_set(1);
        }
        else if((asr_word_idx==2)&&(asr_wakeup_flag==1))
        {
            asr_wakeup_set(0);
        }
        else
        {
            //
            char audio[24];
            sprintf(audio,"%d.mp3",asr_word_idx);
            os_set_play_audio(audio,NULL);
            robot_action_data_saveplay(2,10+asr_word_idx);
            //switch(asr_word_idx)
            //{
            //case 6:
            //    robot_action_data_saveplay(2,10);
            //    break;
            //
            //}

        }
        asr_action_flag = 1;
        asr_word_idx = 0;
        asr_timer_cnt_pause();
    }
}

void asr_aciton_end()
{

    if(asr_action_flag==2)
    {
        //asr_action_flag = 0;
        asr_timer_cnt_clear();
        wanson_asr_lock(0);
        asr_action_flag = 0;
        //extern int asr_wakeup_flag;
        //if(asr_wakeup_flag)
        //    robot_emoji_mic_start2();
    }
}


void asr_word_deal(const char* result_dat)
{
    int cmd_count = 32;
    int item = wanson_get_word_idx(result_dat);
    if (item > cmd_count)
        return;
    DBG2_PRINT("asr_word_deal >> ",item,asr_wakeup_flag);

    wanson_asr_lock(1);
    #if 0
    if(asr_once_flag)//program
    {
        //asr_word_idx = item+1;
        extern int app_program_asrcode;
        app_program_asrcode = item+1;
        return ;
    }
    #endif

    if (asr_wakeup_flag)
    { // 如果在唤醒状态，识别了控制指令则重新计时唤醒时间
        asr_timer_cnt_clear();
    }
    else
    { // 在非唤醒状态，识别非唤醒词则不作处理
        //if(item>1)
        if((item>=3)&&(item<=22))
        {
            asr_word_idx = item;
            return ;
        }
        else
        {
            wanson_asr_lock(0);
            return;
        }
    }
    if(item<=32)//
    {
        asr_word_idx = item;
    }
    else
    {
        wanson_asr_lock(0);
    }
}

extern void asr_once_set(int once);
extern int app_program_asrcode;
void program_asr_listen_start()
{
    int time = 500;
    app_program_asrcode = 0;
    wanson_next_group_set(1);
    asr_once_set(1);
    while(time--)
    {
        if(app_program_asrcode)
            break;
        puppy_delay(10);
    }
    asr_once_set(0);
    wanson_asr_lock(0);
    DBG_PRINT("app_program_asrcode = ",app_program_asrcode);
}

void asr_once_set(int once)
{
    asr_once_flag = once;
}


#define WIDX_CMD_LEN              24 //Please do not modify
#define VOICE_WAKEUP_TIMEOUT        20

typedef struct {
    int widx;
    char cmd[WIDX_CMD_LEN];

}asr_widx_t;


const asr_widx_t cmd_widx[] = {
	{1, "你好小博士"},
	{2, "再见"},
	{2, "不聊了"},
	{3, "你是谁"},
	{3, "你叫什么名字"},
	{4, "你多大了"},
	{4, "你几岁了"},
	{5, "你会做什么"},
	{5, "你有什么本领"},
	{6, "点点头"},
	{7, "摇摇头"},
	{8, "抬抬头"},
	{9, "去睡觉"},
	{10, "睡觉去"},
	{10, "晚安"},
	{11, "做个鬼脸"},
	{11, "做鬼脸"},
	{12, "奸笑"},
	{13, "笑一个"},
	{13, "高兴"},
	{14, "哈哈大笑"},
	{14, "笑哈哈"},
	{15, "哭一个"},
	{16, "难过"},
	{16, "伤心"},
	{16, "沮丧"},
	{17, "怒一个"},
	{17, "生气"},
	{18, "愤怒"},
	{18, "怒气冲冲"},
	{19, "乐一个"},
	{19, "开心"},
	{19, "笑口常开"},
	{20, "向左看"},
	{21, "向右看"},
	{22, "向前看"},
	{23, "大声点"},
	{23, "声音大点"},
	{24, "小声点"},
	{24, "声音小点"},
	{25, "最大音量"},
	{26, "最小音量"},
};


int wanson_get_word_idx(const char* result_dat)
{
    int i,j;
    int sz = sizeof(cmd_widx)/sizeof(asr_widx_t);
    for(i=0;i<sz;i++)
    {
        if(strcmp(cmd_widx[i].cmd,result_dat)==0)
        {
            return cmd_widx[i].widx;
        }
    }
    return 0;
}

#if 0
static unsigned int  asr_wakeup_tick_timer = 0;
int asr_wakeup_tick_enable = 1;

void asr_timer_cnt_pause()
{
    asr_wakeup_tick_enable = 0;
}

void asr_timer_cnt_clear(void)
{
    asr_wakeup_tick_enable = 1;
    asr_wakeup_tick_timer = 0;
}


static void asr_timer_loop(void)
{
    static int cnt = 0;
    cnt++;
    if (cnt >= 10) // 1S
    {
        cnt = 0;
        if (asr_wakeup_flag && asr_wakeup_tick_enable) {
            asr_wakeup_tick_timer++;
        } else {
            asr_wakeup_tick_timer = 0;
        }
        // printf("asr_wakeup_tick_timer >>> %d\r\n", asr_wakeup_tick_timer);
        // extern void aec_mix_data_stip();
        // aec_mix_data_stip();
        // printf("**************************************************stop");
    }
}

void asr_timer_init(void)
{
    asr_wakeup_tick_timer = 0;
    asr_wakeup_tick_enable = 1;
    asr_wakeup_flag = 0;
    //wakeup_flag = 0;

    // priv:私有参数
    // func:定时扫描回调函数
    // msec:定时时间， 单位：毫秒
    sys_s_hi_timer_add(NULL, asr_timer_loop, 100);
}

int32_t asr_is_timeout(unsigned int timeOut)
{
    return asr_wakeup_tick_timer > timeOut ? 1 : 0;
}

//接到ACTION里的唤醒退出
void app_asr_timerout_check()
{
    extern int asr_wakeup_flag;
    if (asr_wakeup_flag && asr_is_timeout(VOICE_WAKEUP_TIMEOUT)) { //退出唤醒
        asr_timer_cnt_clear();
        robot_asr_stop();
    }
}
#endif

#if 0
uint16_t robot_asr_step = 0;
uint16_t robot_asr_word_idx = -1;
uint16_t robot_asr_switch = 0;
char robot_asr_word[64];

extern int Teboz_ASR_Function_Set(int flag);
extern int Teboz_ASR_Robot_StatusFlag_Get();
extern int Teboz_ASR_Recog_Set();
extern int Teboz_BackGroundNoise_Set();

void robot_asr_set_init()
{
    if(robot_asr_switch)
        STRDBG_PRINT("robot_asr_set_init ","Go");
    Teboz_ASR_Set_Init();
}

uint16_t robot_asr_get_ready()//判断是不是已经初始化完了呢。
{
    int asr_status = Teboz_ASR_Robot_StatusFlag_Get();
    //if(robot_asr_switch)
    //    DBG_PRINT("asr_status = ",asr_status);
    return (asr_status==2);
}

void robot_asr_set_recog_once()
{
    if(!robot_asr_get_ready())
    {
        STR_PRINT("ASR Recog Not Ready!\r\n");
        return ;
    }
    robot_asr_word_idx = -1;
    if(Teboz_ASR_Recog_Set())
    {
        STR_PRINT("ASR Recog Start OK\r\n");
        robot_asr_step = 1;
    }
    else
    {
        STR_PRINT("ASR Recog Start Err\r\n");
    }
}

void robot_asr_set_recog_break()
{
    if(Teboz_ASR_RecogBreak_Set())
    {
        STR_PRINT("ASR Recog Break OK\r\n");
        robot_asr_step = 1;
    }
    else
    {
        STR_PRINT("ASR Recog Break Err\r\n");
    }
    if(robot_asr_word_idx)
        robot_asr_word_idx = -1;
}

void robot_asr_word_set(uint16_t widx,char word[])
{
    if(widx>=0)
    {
        robot_asr_word_idx = widx;
        strcpy(robot_asr_word,word);
        robot_asr_step = 2;
        DBG_PRINT("WIDX = ",widx);
        STRDBG_PRINT("WORD = ",word);
    }
}

void robot_asr_function_set(uint16_t flag)
{
    if(!Teboz_ASR_Function_Set(flag))
        DBG_PRINT("ASR_FUNC_SET OK",flag);
    else
        DBG_PRINT("ASR_FUNC_SET ERR",flag);
}

void robot_action_rand_CTT()
{

    int randomValue = rand() % 2;
    DBG_PRINT("randomValue",randomValue);

    switch (randomValue)
    {
        case 0:
            robot_action_data_saveplay(2, 22);
            break;
        case 1:
            robot_action_data_saveplay(2, 19);
            break;
        default:
            break;
    }
}

void robot_action_rand_function()
{
    int randomValue = rand() % 9;
    DBG_PRINT("randomValue",randomValue);

    switch (randomValue)
    {
        case 0:
            robot_action_data_saveplay(2, 25);
            break;
        case 1:
            robot_action_data_saveplay(2, 26);
            break;
        case 2:
            robot_action_data_saveplay(2, 27);
            break;
        case 3:
            robot_action_data_saveplay(2, 28);
            break;
        case 4:
            robot_action_data_saveplay(2, 31);
            break;
        case 5:
            robot_action_data_saveplay(2, 32);
            break;
        case 6:
            robot_action_data_saveplay(2, 33);
            break;
        case 7:
            robot_action_data_saveplay(2, 34);
            break;
        case 8:
            robot_action_data_saveplay(2, 35);
            break;
        default:
            break;
    }
}


void robot_action_rand_ZGGF()
{
    int randomValue = rand() % 7;
    DBG_PRINT("randomValue",randomValue);

    switch (randomValue)
    {
        case 0:
            robot_action_data_saveplay(2, 14);
            break;
        case 1:
            robot_action_data_saveplay(2, 20);
            break;
        case 2:
            robot_action_data_saveplay(2, 21);
            break;
        case 3:
            robot_action_data_saveplay(2, 17);
            break;
        case 4:
            robot_action_data_saveplay(2, 18);
            break;
        case 5:
            robot_action_data_saveplay(2, 22);
            break;
        case 6:
            robot_action_data_saveplay(2, 19);
            break;
        default:
            break;
    }
}

void robot_asr_word_make()
{
    if(robot_asr_word_idx == -1)
        return ;
    if(robot_asr_step !=2 )
        return ;
    robot_asr_step = 3;
    // to do
    switch(robot_asr_word_idx)
    {
        case 0 : robot_action_data_saveplay(2,1);break;
//      case 1 : robot_action_data_saveplay(2,2);break;
        case 3 : robot_action_data_saveplay(2,12);break;//俯卧撑
        case 4 : robot_action_data_saveplay(2,16);break;//前空翻
        case 5 : robot_action_data_saveplay(2,13);break;//后空翻
        case 6 : robot_action_data_saveplay(2,11);break;//匍匐前进
        case 7 : robot_action_data_saveplay(2,15);break;//做劈叉
        case 8 : robot_action_rand_CTT();break;//侧踢腿
        case 9 : robot_action_data_saveplay();break;//金鸡独立
        case 10 : robot_action_rand_function();break;//跳个舞
        case 11 : robot_action_data_saveplay(2,2);break;//前进
        case 12 : robot_action_data_saveplay(2,3);break;//后退
        case 13 : robot_action_data_saveplay(2,4);break;//左转
        case 14: robot_action_data_saveplay(2,5);break;//右转
        case 15: robot_action_rand_function();break;//秀一个
        case 16: robot_action_rand_function();break;//来个绝活
        case 17: robot_action_rand_ZGGF();break;//中国功夫
        default :
            break;
    }
    memset(robot_asr_word,0,sizeof(robot_asr_word));
    robot_asr_word_idx = -1;
    robot_asr_step = 0;
}

void robot_asr_pcmsend_flag_set(uint16_t set)
{

}

void robot_asr_statusflag_set(uint16_t flag)
{
    if(robot_asr_switch)
    {
        int noise;
        DBG_PRINT("asr status flag = ",flag);
        switch(flag)
        {
            case -1:STRDBG_PRINT("ASR_STATUS","INIT ERR");break;
            case 0:STRDBG_PRINT("ASR_STATUS","UN INIT");break;
            case 1:STRDBG_PRINT("ASR_STATUS","INIT GO");break;
            case 2:STRDBG_PRINT("ASR_STATUS","INIT OK");break;
            case 3:STRDBG_PRINT("ASR_STATUS","NOISE SET");break;
            case 4:
                STRDBG_PRINT("ASR_STATUS","NOISE OK");
                noise = Teboz_BackGroundNoise_Get();
                DBG_PRINT("BackGround Noise = ",noise);
                break;
            case 5:STRDBG_PRINT("ASR_STATUS","RECONG GO");break;
            case 6:STRDBG_PRINT("ASR_STATUS","RECONG OK");break;
            case 7:STRDBG_PRINT("ASR_STATUS","INIT DONE");break;
            case 8:STRDBG_PRINT("ASR_STATUS","ASR BUSY");break;
            case 9:STRDBG_PRINT("ASR_STATUS","INIT SET");break;
            case 10:STRDBG_PRINT("ASR_STATUS","RECORD SET");break;
            case 11:STRDBG_PRINT("ASR_STATUS","RECORD DONE");break;
            case 12:STRDBG_PRINT("ASR_RECOG","VOICE IN");break;
            case 13:STRDBG_PRINT("ASR_RECOG","VOICE OUT");break;
            case 14:STRDBG_PRINT("ASR_RECOG","NO VOICE");break;
            case 15:STRDBG_PRINT("ASR_RECOG","CAN NOT STOP");break;
            case 16:STRDBG_PRINT("ASR_RECOG","RECOG BREAK");break;
        }
    }
}
int robot_asr_switch_get()
{
    return robot_asr_switch;
}

void robot_asr_switch_set(uint16_t sw)
{
    robot_asr_switch = sw;
    DBG_PRINT("asr robot_asr_switch = ",sw);
    if(sw)
    {
        int noise,ready;
        noise = Teboz_BackGroundNoise_Get();
        DBG_PRINT("BackGround Noise = ",noise);
        ready = robot_asr_get_ready();
        if(ready)
        {
            STRDBG_PRINT("ASR READY:","OK");
            Tbz_Config_Save(1,sw);
        }
        else
            STRDBG_PRINT("ASR READY:","NOT READY");
    }
}

void robot_asr_record_show_start()
{
    STRDBG_PRINT("RECORD_DATA","START");
}

void robot_asr_record_show_page(uint16_t *buf,uint16_t sz,uint16_t pageid)
{
    DBG_PRINT("PAGE:",pageid);
    HEXS_PRINT("NULL",(uint8_t*)buf,sz*2);
}

void robot_asr_record_show_end()
{
    STRDBG_PRINT("RECORD_DATA","END");
}

void robot_asr_init()
{
    robot_asr_switch_set(1);
    robot_asr_set_init();
}
#endif
