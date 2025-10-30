#include "web_socket/websocket_api.h"
#include "wifi/wifi_connect.h"
#include "system/includes.h"
#include "robot_debug.h"
#include "json_tools.h"  // 添加JSON工具头文件
#include "robot_cmd.h"   // 添加robot命令头文件
#include "xz_main.h"
#include "pcm_play_api.h" // 添加PCM播放API头文件
#include "opus_decoder_wrapper.h" // 添加Opus解码器包装头文件
#define USE_WEBSOCKET_TEST

#ifdef WEBSOCKET_MEMHEAP_DEBUG
#include "mem_leak_test.h"
#endif

#include "app_config.h"

#ifdef USE_WEBSOCKET_TEST
/**** websoket "ws://"  websokets "wss://" *****/

#define OBJ_URL 	"ws://192.168.3.130:8000/xiaozhi/v1/?device-id=123456&client-id=123456"  //远程服务器测试


// 全局session_id变量
char g_session_id[64] = {0};
bool g_session_id_received = false;

// 全局WebSocket实例
static struct websocket_struct *g_websockets_info = NULL;

// 音频播放相关全局变量
static void *g_pcm_player = NULL;  // PCM播放器句柄
static bool g_audio_playing = false;  // 音频播放状态标志
static opus_decoder_handle_t g_opus_decoder = NULL;  // Opus解码器句柄

// Opus音频参数（根据协议定义）
#define OPUS_SAMPLE_RATE    16000
#define OPUS_CHANNELS       1
#define OPUS_FRAME_DURATION 60
#define OPUS_FRAME_SIZE     (OPUS_SAMPLE_RATE * OPUS_FRAME_DURATION / 1000)  // 960 samples
#define PCM_BUFFER_SIZE     (OPUS_FRAME_SIZE * 2 * OPUS_CHANNELS)  // 每帧PCM数据大小（字节）

// 音频播放器初始化
static int audio_player_init(void)
{
    if (g_pcm_player != NULL && g_opus_decoder != NULL) {
        DBG_PRINTF("Audio player already initialized\n");
        return 0;
    }

    // 初始化Opus解码器
    if (g_opus_decoder == NULL) {
        g_opus_decoder = opus_decoder_wrapper_init(OPUS_SAMPLE_RATE, OPUS_CHANNELS);
        if (g_opus_decoder == NULL) {
            DBG_PRINTF("Failed to initialize opus decoder\n");
            return -1;
        }
        DBG_PRINTF("Opus decoder initialized\n");
    }

    // 打开PCM播放器
    // 参数：采样率、帧大小、丢弃点数、声道数、音量、阻塞模式
    if (g_pcm_player == NULL) {
        g_pcm_player = audio_pcm_play_open(
            OPUS_SAMPLE_RATE,    // 采样率 16000Hz
            PCM_BUFFER_SIZE * 4, // 帧缓冲大小
            0,                   // 丢弃点数
            OPUS_CHANNELS,       // 声道数 1
            80,                  // 音量 0-100
            0                    // 非阻塞模式
        );

        if (g_pcm_player == NULL) {
            DBG_PRINTF("Failed to open PCM player\n");
            opus_decoder_wrapper_destroy(g_opus_decoder);
            g_opus_decoder = NULL;
            return -1;
        }

        // 启动播放器
        if (audio_pcm_play_start(g_pcm_player) != 0) {
            DBG_PRINTF("Failed to start PCM player\n");
            audio_pcm_play_stop(g_pcm_player);
            g_pcm_player = NULL;
            opus_decoder_wrapper_destroy(g_opus_decoder);
            g_opus_decoder = NULL;
            return -1;
        }
    }

    g_audio_playing = true;
    DBG_PRINTF("Audio player initialized successfully\n");
    return 0;
}

// 音频播放器停止
static void audio_player_stop(void)
{
    if (g_pcm_player != NULL) {
        audio_pcm_play_stop(g_pcm_player);
        g_pcm_player = NULL;
    }
    
    if (g_opus_decoder != NULL) {
        opus_decoder_wrapper_destroy(g_opus_decoder);
        g_opus_decoder = NULL;
    }
    
    g_audio_playing = false;
    DBG_PRINTF("Audio player stopped\n");
}

// 播放PCM数据
static int audio_play_pcm_data(u8 *pcm_data, u32 size)
{
    if (g_pcm_player == NULL) {
        DBG_PRINTF("PCM player not initialized\n");
        return -1;
    }

    int ret = audio_pcm_play_data_write(g_pcm_player, pcm_data, size);
    if (ret < 0) {
        DBG_PRINTF("Failed to write PCM data\n");
        return -1;
    }

    return 0;
}

static void websockets_callback(u8 *buf, u32 len, u8 type)
{
    DBG_PRINTF("wbs recv type: %u, len: %u\n", type, len);

    // type = 129 (0x81): JSON文本数据
    // type = 130 (0x82): 二进制Opus数据
    if (type == 129) {
        // 处理JSON消息
        DBG_PRINTF("wbs recv JSON msg: %s\n", buf);
        websocket_message_t message;
        if (parse_websocket_message((char*)buf, &message)) {
            DBG_PRINTF("Received JSON message type: %s\n", message_type_to_string(message.type));

            // 根据消息类型处理
            switch (message.type) {
                case MSG_TYPE_HELLO:
                    DBG_PRINTF("Hello message received, version: %d\n", message.version);
                    // 保存session_id
                    if (strlen(message.session_id) > 0) {
                        strncpy(g_session_id, message.session_id, sizeof(g_session_id) - 1);
                        g_session_id_received = true;
                        DBG_PRINTF("Session ID saved: %s\n", g_session_id);
                    }
                    break;
                case MSG_TYPE_STT:
                    DBG_PRINTF("STT text: %s\n", message.data.stt.text);
                    break;
                case MSG_TYPE_TTS:
                    DBG_PRINTF("TTS state: %d, text: %s\n", message.data.tts.state, message.data.tts.text);
                    
                    // TTS开始时初始化音频播放器
                    if (message.data.tts.state == TTS_STATE_START) {
                        if (audio_player_init() == 0) {
                            DBG_PRINTF("Audio player initialized for TTS playback\n");
                        }
                    }
                    // TTS结束时停止音频播放器
                    else if (message.data.tts.state == TTS_STATE_END) {
                        audio_player_stop();
                        DBG_PRINTF("Audio player stopped after TTS end\n");
                    }
                    break;
                case MSG_TYPE_LISTEN:
                    DBG_PRINTF("Listen state: %d, mode: %d\n", message.data.listen.state, message.data.listen.mode);
                    break;
                default:
                    DBG_PRINTF("Unhandled message type\n");
                    break;
            }

            free_websocket_message(&message);
        }
    }
    else if (type == 130) {
        // 处理二进制Opus音频数据
        DBG_PRINTF("Received Opus audio data, len: %u\n", len);
        
        // 确保音频播放器已初始化
        if (!g_audio_playing) {
            DBG_PRINTF("Audio player not ready, initializing...\n");
            if (audio_player_init() != 0) {
                DBG_PRINTF("Failed to initialize audio player\n");
                return;
            }
        }

        // 解码Opus数据为PCM
        if (g_opus_decoder != NULL) {
            // 分配PCM输出缓冲区
            s16 pcm_output[OPUS_FRAME_SIZE * OPUS_CHANNELS];
            
            // 调用opus解码器
            int decoded_samples = opus_decoder_wrapper_decode(
                g_opus_decoder, 
                buf, 
                len, 
                pcm_output, 
                OPUS_FRAME_SIZE
            );
            
            if (decoded_samples > 0) {
                // 计算PCM数据字节数（采样点数 * 2字节/采样点 * 声道数）
                u32 pcm_size = decoded_samples * 2 * OPUS_CHANNELS;
                
                // 播放解码后的PCM数据
                if (audio_play_pcm_data((u8*)pcm_output, pcm_size) == 0) {
                    DBG_PRINTF("Played PCM data: %d samples, %d bytes\n", decoded_samples, pcm_size);
                } else {
                    DBG_PRINTF("Failed to play PCM data\n");
                }
            } else {
                DBG_PRINTF("Failed to decode opus data: %d\n", decoded_samples);
            }
        } else {
            DBG_PRINTF("Opus decoder not initialized\n");
        }
    }
    else {
        DBG_PRINTF("Unknown data type: %u\n", type);
    }
}



/*******************************************************************************
*   Websocket Client api
*******************************************************************************/
static void websockets_client_reg(struct websocket_struct *websockets_info, char mode)
{
    memset(websockets_info, 0, sizeof(struct websocket_struct));
    websockets_info->_init           = websockets_client_socket_init;
    websockets_info->_exit           = websockets_client_socket_exit;
    websockets_info->_handshack      = webcockets_client_socket_handshack;
    websockets_info->_send           = websockets_client_socket_send;
    websockets_info->_recv_thread    = websockets_client_socket_recv_thread;
    websockets_info->_heart_thread   = websockets_client_socket_heart_thread;
    websockets_info->_recv_cb        = websockets_callback;
    websockets_info->_recv           = NULL;

    websockets_info->websocket_mode  = mode;
}

static int websockets_client_init(struct websocket_struct *websockets_info, u8 *url, const char *origin_str)
{
    websockets_info->ip_or_url = url;
    websockets_info->origin_str = origin_str;
    websockets_info->recv_time_out = 1000;

    int err = websockets_struct_check(sizeof(struct websocket_struct));
    if (err == FALSE) {
        return err;
    }
    return websockets_info->_init(websockets_info);
}

static int websockets_client_handshack(struct websocket_struct *websockets_info)
{
    DBG_PRINTF("Starting handshake with server: %s\n", websockets_info->ip_or_url);
    int result = websockets_info->_handshack(websockets_info);
    DBG_PRINTF("Handshake result: %d\n", result);
    return result;
}

static int websockets_client_send_json(struct websocket_struct *websockets_info, cJSON *json_msg)
{
    if (!json_msg) {
        DBG_PRINTF("JSON message is NULL\n");
        return FALSE;
    }

    // 转换为JSON字符串（无格式，节省空间）
    char* json_str = cJSON_PrintUnformatted(json_msg);
    if (!json_str) {
        DBG_PRINTF("Failed to convert JSON to string\n");
        return FALSE;
    }

    DBG_PRINTF("Sending JSON message: %s\n", json_str);

    // 发送消息
    int result = websockets_info->_send(websockets_info, (u8*)json_str, strlen(json_str), WCT_TXTDATA);

    // 释放内存
    free(json_str);

    return result;
}

static void websockets_client_exit(struct websocket_struct *websockets_info)
{
    websockets_info->_exit(websockets_info);
}

/*******************************************************************************
*   创建并发送各种JSON消息的函数
*******************************************************************************/
static int send_hello_message(struct websocket_struct *websockets_info)
{
    // 创建Hello消息
    features_t features = { .mcp = true };
    audio_params_t audio_params = {
        .format = "opus",
        .sample_rate = 16000,
        .channels = 1,
        .frame_duration = 60
    };

    cJSON* hello_msg = create_hello_message(1, &features, "websocket", &audio_params);
    int result = websockets_client_send_json(websockets_info, hello_msg);
    cJSON_Delete(hello_msg);

    return result;
}

static int send_listen_message(struct websocket_struct *websockets_info,
                              listen_state_t state,
                              listen_mode_t mode,
                              const char* text)
{
    if (!g_session_id_received) {
        DBG_PRINTF("Error: No session ID received yet\n");
        return FALSE;
    }

    cJSON* listen_msg = create_listen_message(g_session_id, state, mode, text);
    int result = websockets_client_send_json(websockets_info, listen_msg);
    cJSON_Delete(listen_msg);
    return result;
}

static int send_abort_message(struct websocket_struct *websockets_info,
                              const char* reason)
{
    if (!g_session_id_received) {
        DBG_PRINTF("Error: No session ID received yet\n");
        return FALSE;
    }

    cJSON* abort_msg = create_abort_message(g_session_id, reason);
    int result = websockets_client_send_json(websockets_info, abort_msg);
    cJSON_Delete(abort_msg);
    return result;
}

static int send_stt_message(struct websocket_struct *websockets_info,
                           const char* text)
{
    if (!g_session_id_received) {
        DBG_PRINTF("Error: No session ID received yet\n");
        return FALSE;
    }

    cJSON* stt_msg = create_stt_message(g_session_id, text);
    int result = websockets_client_send_json(websockets_info, stt_msg);
    cJSON_Delete(stt_msg);
    return result;
}

static int send_tts_message(struct websocket_struct *websockets_info,
                          tts_state_t state,
                          const char* text)
{
    if (!g_session_id_received) {
        DBG_PRINTF("Error: No session ID received yet\n");
        return FALSE;
    }

    cJSON* tts_msg = create_tts_message(g_session_id, state, text);
    int result = websockets_client_send_json(websockets_info, tts_msg);
    cJSON_Delete(tts_msg);
    return result;
}
static int send_mcp_message(struct websocket_struct *websockets_info,
                           cJSON* payload)
{
    if (!g_session_id_received) {
        DBG_PRINTF("Error: No session ID received yet\n");
        return FALSE;
    }

    cJSON* mcp_msg = create_mcp_message(g_session_id, payload);
    int result = websockets_client_send_json(websockets_info, mcp_msg);
    cJSON_Delete(mcp_msg);
    return result;
}

//发送字符串数据
static int send_tag_message(struct websocket_struct *websockets_info, const char* tag_text)
{
    if (!tag_text) {
        DBG_PRINTF("Error: Tag text is NULL\n");
        return FALSE;
    }

    // 使用现有的listen消息类型，
    cJSON* tag_msg = create_listen_message(g_session_id, LISTEN_STATE_START,
                                          LISTEN_MODE_AUTO, tag_text);

    if (!tag_msg) {
        DBG_PRINTF("Failed to create tag message\n");
        return FALSE;
    }

    // 转换为JSON字符串
    char* json_str = cJSON_PrintUnformatted(tag_msg);
    if (!json_str) {
        DBG_PRINTF("Failed to convert tag message to JSON string\n");
        cJSON_Delete(tag_msg);
        return FALSE;
    }

    DBG_PRINTF("Sending tag message: %s\n", json_str);

    // 发送消息
    int result = websockets_info->_send(websockets_info, (u8*)json_str, strlen(json_str), WCT_TXTDATA);

    // 释放内存
    free(json_str);
    cJSON_Delete(tag_msg);

    return result;
}
void send_tag_to_websocket(const char* tag_text)
{
    if (!g_websockets_info) {
        DBG_PRINTF("WebSocket not connected, cannot send tag message\n");
        return;
    }
    if (!g_session_id_received) {
        DBG_PRINTF("Error: No session ID received yet\n");
        return;
    }
    int result = send_tag_message(g_websockets_info, tag_text);
    if (result == FALSE) {
        DBG_PRINTF("Failed to send tag message via WebSocket\n");
    } else {
        DBG_PRINTF("Tag message sent successfully: %s\n", tag_text);
    }
}



/*******************************************************************************
*   Websocket Client主线程
*******************************************************************************/
static void websockets_client_main_thread(void *priv)
{
    int err;
    char mode = WEBSOCKET_MODE;
#ifdef WIN32
    void *heart_thread_hdl = NULL;
    void *recv_thread_hdl = NULL;
#endif
    u8 url[256] = OBJ_URL;
    const char *origin_str = "http://coolaf.com";

    DBG_PRINTF(" . ----------------- Client Websocket ------------------\r\n");

#ifdef WIN32
    DBG_PRINTF(" . Please input the mode for websocket : 1 . websocket , 2 . websockets (ssl)\r\n");
    scanf("%d", &mode);
    if (mode == 2) {
        mode = WEBSOCKETS_MODE;
        DBG_PRINTF(" . Please input the URL , for example: wss://localhost:8888\r\n");
    } else {
        mode = WEBSOCKETS_MODE;
        DBG_PRINTF(" . Please input the URL , for example: ws://localhost:8888\r\n");
    }
    scanf("%d", &url);
#endif

    /* 0 . 分配内存 */
    struct websocket_struct *websockets_info = malloc(sizeof(struct websocket_struct));
    if (!websockets_info) {
        return;
    }

    // 赋值给全局变量
    g_websockets_info = websockets_info;

    /* 1 . 注册WebSocket函数 */
    websockets_client_reg(websockets_info, mode);

    /* 2 . 初始化WebSocket */
    err = websockets_client_init(websockets_info, url, origin_str);
    if (FALSE == err) {
        DBG_PRINTF("  . ! Client websocket init error !!!\r\n");
        goto exit_ws;
    }

    /* 3 . 握手 */
    err = websockets_client_handshack(websockets_info);
    if (FALSE == err) {
        DBG_PRINTF("  . ! Handshake error !!!\r\n");
        goto exit_ws;
    }
    DBG_PRINTF(" . Handshake success \r\n");

    /* 4 . 创建心跳和接收线程 */
#ifdef WIN32
    heart_thread_hdl = CreateThread(NULL, 0, websockets_info->_heart_thread, websockets_info, 0, &websockets_info->ping_thread_id);
    recv_thread_hdl = CreateThread(NULL, 0, websockets_info->_recv_thread,  websockets_info, 0, &websockets_info->ping_thread_id);
#else
    thread_fork("websocket_client_heart", 19, 512, 0,
                &websockets_info->ping_thread_id,
                websockets_info->_heart_thread,
                websockets_info);
    thread_fork("websocket_client_recv", 18, 512, 0,
                &websockets_info->recv_thread_id,
                websockets_info->_recv_thread,
                websockets_info);
#endif
    websockets_sleep(1000);

    /* 5 . 发送Hello消息 */
    err = send_hello_message(websockets_info);
    if (FALSE == err) {
        DBG_PRINTF("  . ! Send hello message error !!!\r\n");
        goto exit_ws;
    }
    DBG_PRINTF("  . Hello message sent successfully \r\n");

    /* 等待收到服务器的session_id */
    DBG_PRINTF("Waiting for server session ID...\n");
    int wait_count = 0;
    while (!g_session_id_received && wait_count < 50) { // 最多等待5秒
        websockets_sleep(100);
        wait_count++;
    }


    if (!g_session_id_received) {
        DBG_PRINTF("Error: Timeout waiting for session ID\n");
        goto exit_ws;
    }
    DBG_PRINTF("Session ID received: %s, starting message loop\n", g_session_id);

    /* 6 . 主循环 - 发送各种JSON消息 */
    int message_count = 0;
    while (1) {
        message_count++;

        // 根据计数发送不同类型的消息
        switch (message_count % 4) {
            case 0:
                // 发送监听开始消息 TYPY:listen start
                err = send_listen_message(websockets_info,
                                         LISTEN_STATE_START, LISTEN_MODE_AUTO, NULL);
                DBG_PRINTF("Sent listen start message\n");
                break;
            case 1:
                // 发送监听停止消息 TYPY:listen stop
                err = send_listen_message(websockets_info,
                                         LISTEN_STATE_STOP, LISTEN_MODE_AUTO, "Stop message");
                DBG_PRINTF("Sent listen stop message\n");
                break;
            case 2:
                // 发送监听唤醒消息 TYPY:listen detect   //这个需要有音频数据 不然会有bug 提示错误：请正确配置 OTA地址，然后重新编译固件。
                err = send_listen_message(websockets_info,
                                         LISTEN_STATE_DETECT, LISTEN_MODE_AUTO, "小明小明");
                DBG_PRINTF("Sent listen detect message\n");
                break;
            case 3:
                // 发送终止消息 TYPY:abort
                err = send_abort_message(websockets_info, "wake_word_detected");
                DBG_PRINTF("Sent Abort message\n");
                break;
//            case 1:
//                // 发送STT消息（模拟语音识别结果）
//                err = send_stt_message(websockets_info,
//                                      "你好，这是一个测试消息");
//                DBG_PRINTF("Sent STT message\n");
//                break;

//            case 2:
//                // 发送TTS开始消息
//                err = send_tts_message(websockets_info,
//                                      TTS_STATE_START, NULL);
//                DBG_PRINTF("Sent TTS start message\n");
//                break;

//            case 3:
//                // 发送监听停止消息
//                err = send_listen_message(websockets_info,
//                                         LISTEN_STATE_STOP, LISTEN_MODE_AUTO, NULL);
//                DBG_PRINTF("Sent listen stop message\n");
//                break;
        }

        if (FALSE == err) {
            DBG_PRINTF("  . ! Send message error !!!\r\n");
            goto exit_ws;
        }

        websockets_sleep(5000); // 每5秒发送一条消息

#ifdef WIN32
        // Windows下的交互测试
        if (message_count % 3 == 0) {
            DBG_PRINTF("Press Enter to continue or 'q' to quit...\n");
            char input = getchar();
            if (input == 'q' || input == 'Q') {
                break;
            }
        }
#endif
    }

exit_ws:
    /* 7 . 清理退出 */
#ifdef WIN32
    if (heart_thread_hdl) TerminateThread(heart_thread_hdl, 0);
    if (recv_thread_hdl) TerminateThread(recv_thread_hdl, 0);
#else
    thread_kill(&websockets_info->ping_thread_id, KILL_REQ);
    thread_kill(&websockets_info->recv_thread_id, KILL_REQ);
#endif

    websockets_client_exit(websockets_info);
    free(websockets_info);
}

static void websocket_client_thread_create(void)
{
    thread_fork("websockets_client_main", 15, 512 * 3, 0, 0, websockets_client_main_thread, NULL);
}

static void http_websocket_start(void *priv)
{
    enum wifi_sta_connect_state state;
    while (1) {
        DBG_PRINTF("Connecting to the network...\n");
        state = wifi_get_sta_connect_state();
        if (WIFI_STA_NETWORK_STACK_DHCP_SUCC == state) {
            DBG_PRINTF("Network connection is successful!\n");
            break;
        }
        os_time_dly(500);
    }

    websocket_client_thread_create();
}

//应用程序入口
void c_main(void *priv)
{
    DBG_PRINTF("[c_main] WebSocket JSON Client Starting...\n");
    if (thread_fork("http_websocket_start", 10, 512, 0, NULL, http_websocket_start, NULL) != OS_NO_ERR) {
        DBG_PRINTF("thread fork fail\n");
    }
    DBG_PRINTF("[c_main] WebSocket JSON Client Started\n");
}

late_initcall(c_main);

#endif//USE_WEBSOCKET_TEST
