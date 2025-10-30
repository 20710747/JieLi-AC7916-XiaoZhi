#include "robot_main.h"
#include "robot_uart.h"
#include "robot_debug.h"
#include "robot_control.h"
#include "robot_groupshow.h"
#include "robot_app.h"
#include "robot_asr.h"
#include "robot_storage.h"
#include "robot_action.h"
#include "robot_key.h"
#include "robot_motor.h"
#include "robot_light.h"
#include "robot_pwm.h"
#include "robot_config.h"
#include "robot_adapter.h"

#define ROBOT_UART_DEBUG 0



int RobotMainInputMode = MAININPUT_MODE_CMD;

int Robot_Vbat_level = 0;

int robot_main_breathe_cs = 0;

int ifly_music_flag = 0;
int tbz_next_asr_flag = 0;

int robot_debug_flag = 0;
extern int Config_Init;

OS_SEM robot_main_sem;
int robot_main_sem_flag = 0;
int robot_main_sem_ok = 0;

int robot_key_use = 1;

void robot_key_use_set(int use)
{
    robot_key_use = use;
    DBG_PRINTF("robot_key_use = %d",robot_key_use);
}

int robot_key_use_get()
{
    return robot_key_use;
}
void robot_main_sem_creat()
{
    os_sem_create(&robot_main_sem, 0);
    robot_main_sem_flag = 1;
}

void robot_main_sem_send()
{
    DBG_PRINT("robot_main_sem_send",0);
    robot_main_sem_ok = 1;
    if(robot_main_sem_flag)
    {
        os_sem_set(&robot_main_sem, 0);
        os_sem_post(&robot_main_sem);
    }
}

void robot_main_sem_pend()
{
    DBG_PRINT("robot_main_sem_pend",robot_main_sem_ok);
    if(robot_main_sem_ok)
    {
        return ;
    }
    if(robot_main_sem_flag)
    {
        os_sem_pend(&robot_main_sem, 0);
    }
    DBG_PRINT("robot_main_sem_pend",1);
}

int Robot_Vbat_level_Set(int level)
{
    Robot_Vbat_level = level;
    DBG_PRINT("Robot_Vbat_level_Set = ",level);
}


#if 0
// uart input && ouput

char debug_string[256];
void robot_main_data_output(void *data,uint16_t sz)
{
    if(__robot_main->uart_dev && data && sz)
        dev_write(__robot_main->uart_dev, data, sz);
}

void robot_main_str_output(char *str,uint16_t sz)
{
    #if CONFIG_DEBUG_CHANEL==0
        printf("TBZ LOG : %s",str);
    #else
    if((!sz)&&(str))
        sz = strlen(str);
    if(__robot_main->uart_dev && str && sz)
    {
        dev_write(__robot_main->uart_dev, str, sz);
    }
    #endif
}


void robot_main_ints_print(const char *tag, int count, ...) {
    //sprintf(debug_string,"tag = %s count = %d\r\n",tag,count);
    //robot_main_str_output(debug_string,0);
    //return ;
    char *buf = debug_string;
    int pos = 0;          // 当前写入位置

    // 1. 添加tag
    int n = snprintf(buf, sizeof(buf), "%s ", tag);
    if (n > 0) pos = n;

    va_list args;
    va_start(args, count);  // 初始化可变参数列表

    // 2. 添加所有整数
    for (int i = 0; i < count; i++) {
        int num = va_arg(args, int);  // 获取下一个整数

        if (i == 0) {
            // 第一个数字
            n = snprintf(buf + pos, sizeof(buf) - pos, "%d", num);
        } else {
            // 后续数字添加逗号和空格
            n = snprintf(buf + pos, sizeof(buf) - pos, ", %d", num);
        }

        if (n < 0) break;  // 写入出错
        pos += n;
        if (pos >= (int)sizeof(buf)) break;  // 缓冲区已满
    }
    va_end(args);  // 清理可变参数列表

    // 3. 添加结束标记
    if (pos < (int)sizeof(buf) - 4) {
        strcat(buf, " #\r\n");  // 安全添加结束标记
    } else {
        // 缓冲区几乎已满，直接截断并添加结束符
        buf[sizeof(buf) - 5] = ' ';
        buf[sizeof(buf) - 4] = '#';
        buf[sizeof(buf) - 3] = '\r';
        buf[sizeof(buf) - 2] = '\n';
        buf[sizeof(buf) - 1] = '\0';
    }

    // 4. 调用输出函数
    robot_main_str_output(buf,0);
}

void robot_main_debug_output(char *tag,int32_t data)
{
    //char temp[128];
    sprintf(debug_string,"%s %d #\r\n",tag,data);
    robot_main_str_output(debug_string,0);
    //if(robot_debug_flag>0)
    //    robot_debug_flag--;
}

void robot_main_debug_output_uint32(char *tag, uint32_t data) {
    //char temp[128];
    // 注意：%u 用于无符号整数的格式化输出
    sprintf(debug_string, "%s %u #\r\n", tag, data);
    robot_main_str_output(debug_string, 0);
}

void robot_main_debug_output_str(char *tag, char *data) {
    //char temp[256]; // 增加长度以容纳更长的字符串
    sprintf(debug_string, "%s %s #\r\n", tag, data);
    robot_main_str_output(debug_string, 0);
}

void robot_main_udebug_output(char *tag,uint16_t data)
{
    //char temp[128];
    sprintf(debug_string,"%s %u #\r\n",tag,data);
    robot_main_str_output(debug_string,0);
}

void robot_main_debugf64_output(char *tag,double data)
{
    //char temp[256];
    int data_int = (int)data;
    int data_dec = (int)((data-data_int)*10000);
    sprintf(debug_string,"%s %d.%04d #\r\n",tag,data_int,data_dec);
    robot_main_str_output(debug_string,0);
}

void robot_main_debugu32_output(char *tag,uint32_t data)
{
    //char temp[256];
    sprintf(debug_string,"%s %d(0x%x) #\r\n",tag,data,data);
    robot_main_str_output(debug_string,0);
}


void robot_main_debug32_output(char *tag,int32_t data)
{
    sprintf(debug_string,"%s %d(0x%x) #\r\n",tag,data,data);
    robot_main_str_output(debug_string,0);
}


void robot_main_debug2_output(char *tag,int32_t data1,int32_t data2)
{
    sprintf(debug_string,"%s %d %d#\r\n",tag,data1,data2);
    robot_main_str_output(debug_string,0);
}

void robot_main_debugs_output(char *tag,int16_t* data,uint16_t sz)
{
    int i;
    //char temp[128];
    sprintf(debug_string,"%s :\r\n",tag);
    robot_main_str_output(debug_string,0);
    for(i=0;i<sz;i++)
    {
        sprintf(debug_string,"  %4d",data[i]);
        robot_main_str_output(debug_string,0);
    }
    robot_main_str_output("  #\r\n",0);
}

void robot_main_hex_output(char *tag,uint8_t hex)
{
    //char temp[128];
    sprintf(debug_string,"%s 0x%02x #\r\n",tag,hex);
    robot_main_str_output(debug_string,0);
}

void robot_main_hexs_output(char *tag,uint8_t* data,uint16_t sz)
{
    int i;
    //char temp[128];
    if(strcmp(tag,"NULL") && strcmp(tag,"null"))
    {
        if(sz>=8)
            sprintf(debug_string,"%s[%d] :\r\n",tag,sz);
        else
            sprintf(debug_string,"%s[%d] :",tag,sz);
        robot_main_str_output(debug_string,0);
    }
    for(i=0;i<sz;i++)
    {
        sprintf(debug_string,"%02x ",data[i]);
        robot_main_str_output(debug_string,0);
        if((i+1)%24==0)
            robot_main_str_output("#\r\n",0);
    }
    if(i%24)
        robot_main_str_output("#\r\n",0);
}

void robot_main_debug_string_output(char *tag,char* data)
{
    //char temp[512];
    sprintf(debug_string,"%s %s #\r\n",tag,data);
    robot_main_str_output(debug_string,0);
}

void robot_main_debug_string_output2(char *tag,char* data)
{
    robot_main_str_output(tag,0);
    robot_main_str_output(" ",1);
    robot_main_str_output(data,0);
    robot_main_str_output(" #\r\n",4);
}

#endif

#if 0
int robot_uart_bus_debug = 0;
extern int ts15ctl_makeflag;
void robot_uart_bus_debug_set(int debug)
{
    if(robot_uart_bus_debug!=2)//设置成2之后，再也不能设置回来咯。
        robot_uart_bus_debug = debug;
}

void robot_bus_data_send(void *data,uint16_t sz)
{
    //if(1)
    //{
    //    gpio_direction_output(IO_PORTD_02, 0);    //    1    0      RXD
    //    gpio_direction_output(IO_PORTD_03, 1);    //    0    1      TXD
    //}
    if(__robot_bus->uart_dev && data && sz)
    {
        //if((robot_uart_bus_debug)||(ts15ctl_makeflag == 0))
        //HEXS_PRINT("BUS",data,sz);
        dev_write(__robot_bus->uart_dev, data, sz);
    }
    //if(1)
    //{
    //    gpio_direction_output(IO_PORTD_02, 1);    //    1    0      RXD
    //    gpio_direction_output(IO_PORTD_03, 0);    //    0    1      TXD
    //}
}

//0 in 1 out
void robot_bus_direct_set(int dir)
{
    if(dir)
    {
        gpio_direction_output(IO_PORTD_02, 0);    //    1    0      RXD
        gpio_direction_output(IO_PORTD_03, 1);    //    0    1      TXD
    }
    else
    {
        gpio_direction_output(IO_PORTD_02, 1);    //    1    0      RXD
        gpio_direction_output(IO_PORTD_03, 0);    //    0    1      TXD
    }

}
void robot_bus_data_read_end()
{
    gpio_direction_output(IO_PORTD_02, 0);    //    1    0      RXD
    gpio_direction_output(IO_PORTD_03, 1);    //    0    1      TXD
}

#endif

int robot_main__getCMD(u8* data,int sz)
{
    int i;
    if(sz<2)
        return -1;
    for(i=0;i<=0x0F;i++)
    {
        if((data[0]==0xf0+i)&&(data[1]==i*0x10+0x0f))
        {
            return i;
        }
    }
    return -1;
}

#if 0

uint16_t robot_2p4g_online()
{
    return (__robot_rf2p4g->uart_dev!=NULL);
}

uint16_t robot_main_rf2p4g_testflag()
{
    return (RobotMainInputMode == MAININPUT_MODE_RF2P4G);
}

#endif


extern void robot_main_servo_init();
void robot_main_input_mode_set(uint16_t mode)
{
    if(RobotMainInputMode == MAININPUT_MODE_MOTOR)
        robot_motor_exit();
    if(mode==MAININPUT_MODE_RF2P4G)
        robot_2p4g_switch_set(1);
    if(mode==MAININPUT_MODE_SERVO)
        robot_action_th01_init();
    RobotMainInputMode = mode;
    robot_main_debug_output("Set RobotMainUartMode = ",mode);
}

//actionswitch
int audio_test_idx = 1;
int ws_asr_use = 0;
int tts_speak_flag = 0;
//int tts_speak_debug = 1;
//int tts_speak_debug_idx = 0;
extern int dingflag;
int asr_nodingstep1 = 0;
int asr_nodingstep2 = 0;
//int asr_nodingstep3 = 0;

u32 robot_ai_aiui_reset_time = 0;
int robot_ai_aiui_reset_set = 0;
int robot_ai_aiui_debug = 0;
extern int robot_action_neck_flag;

extern int mouth_debug_mode;
extern int mouth_debug_idx;
extern int mouth_debug_next;
int robot_vcn_idx = 0;


extern int auto_action_step;

int mouth_debug_mode = 0;
int mouth_debug_idx = 0;
int mouth_debug_next = 0;

int speak_face_flag = 1;
int speak_eye_flag = 1;
int speak_mouth_flag = 1;
int speak_wink_flag = 1;
int speak_tone_flag = 1;







#if 0
// 指令信息结构体
typedef struct {
    const char *command;      // 指令字符串
    int param_count;          // 参数个数
    void (*func)(char **);    // 对应的函数指针
} CommandInfo;


//*** 指令加在这里 ***
void robot_command_smile(char **tokens) {
    int para0 = atoi(tokens[1]);
    //printf("执行 smile: 参数=%d\n", para0);
    robot_smile_send(para0);
}

void robot_command_blink(char **tokens) {
    int para0 = atoi(tokens[1]);
    int para1 = atoi(tokens[2]);
    //printf("执行 blink: 参数1=%d, 参数2=%d\n", para0, para1);
    robot_blink_send(para0, para1);
}

void robot_command_rub(char **tokens) {
    int para0 = atoi(tokens[1]);
    int para1 = atoi(tokens[2]);
    int para2 = atoi(tokens[3]);
    //printf("执行 rub: 参数1=%d, 参数2=%d, 参数3=%d\n", para0, para1, para2);
    robot_rub_send(para0, para1, para2);
}

void robot_command_lick(char **tokens) {
    int para0 = atoi(tokens[1]);
    int para1 = atoi(tokens[2]);
    //printf("执行 lick: 参数=%d\n", para0);
    robot_lick_send(para0,para1);
}

void robot_command_speak(char **tokens) {
    int para0 = atoi(tokens[1]);
    int para1 = atoi(tokens[2]);
    int para2 = atoi(tokens[3]);
    //printf("执行 speak: 参数1=%d, 参数2=%d, 参数3=%d\n", para0, para1, para2);
    robot_speak_send(para0, para1, para2);
}

void robot_command_sleep(char **tokens) {
    int para0 = atoi(tokens[1]);
    //printf("执行 sleep: 参数=%d\n", para0);
    robot_sleep_send(para0);
}

void robot_command_speakn(char **tokens) {
    int para0 = atoi(tokens[1]);
    //printf("执行 sleep: 参数=%d\n", para0);
    robot_speakn_send(para0);
}

void robot_command_readservo(char **tokens) {
    robot_action_ts15_posi_read();
}

void robot_command_modeset(char **tokens) {
    int para0 = atoi(tokens[1]);
    robot_main_input_mode_set(para0);
}

void robot_command_headuse(char **tokens) {
    int para0 = atoi(tokens[1]);
    robot_head_use_set(para0);
}

void robot_command_keyuse(char **tokens) {
    int para0 = atoi(tokens[1]);
    //robot_head_use_set(para0);
    robot_key_use_set(para0);
}


//***********************

// 指令表
CommandInfo command_table[] = {
    {"smile", 1, robot_command_smile},
    {"blink", 2, robot_command_blink},
    {"rub", 3, robot_command_rub},
    {"lick", 2, robot_command_lick},
    {"speak", 3, robot_command_speak},
    {"sleep", 1, robot_command_sleep},
    {"speakn", 1, robot_command_speakn},
    {"headuse", 1, robot_command_headuse},
    {"touchuse", 1, robot_command_keyuse},
    {"readservo", 0, robot_command_readservo},
    {"modeset", 1, robot_command_modeset},
    // 可以轻松添加新指令
    // {"new_command", 2, robot_command_new},
};
// 获取指令表中的指令数量
#define COMMAND_COUNT (sizeof(command_table) / sizeof(CommandInfo))

void robot_head_command_send(char *command) {
    char *token = strtok(command, " ");
    char *tokens[100];  // 存储解析后的令牌
    int token_count = 0;

    // 分割字符串
    while (token != NULL && token_count < 100) {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }

    // 解析令牌并执行指令
    for (int i = 0; i < token_count; ) {
        int found = 0;
        // 在指令表中查找匹配的指令
        STRDBG_PRINT("token : ", tokens[i]);

        for (int j = 0; j < COMMAND_COUNT; j++) {
            if (strcmp(tokens[i], command_table[j].command) == 0) {
                // 检查参数数量是否足够
                if (i + command_table[j].param_count < token_count) {
                    // 调用对应的函数，传递tokens数组中当前位置的指针
                    command_table[j].func(&tokens[i]);
                    i += command_table[j].param_count + 1;  // 跳过指令和参数
                    found = 1;
                    break;
                } else {
                    STRDBG_PRINT("Err: No Enougth Para >> ", command_table[j].command);
                    return;  // 参数不足，退出处理
                }
            }
        }
        // 如果未找到匹配的指令，跳过当前token
        if (!found) {
            STRDBG_PRINT("Err: Unknow Command >> ", tokens[i]);
            i++;
        }
    }
}
#endif
//robotcmd
void robot_main_inputGo_TEST(int cmd)
{
    uint16_t touch_status[5];
    switch(cmd)
    {
    case 'a':
        //extern int ifly_music_flag;
        //ifly_music_flag = 0;
        //parseIvwResultTest(1);
        //STRDBG_PRINT("devid = ",aiui_get_devid());
        //debug_show_set();
        robot_head_use_set(0);
        robot_uart_bus_debug_set(2);
        break;
    case 'b':
        //robot_audio_play_break();
        //extern void ifly_aiui_cancel_recognize_process();
        //ifly_aiui_cancel_recognize_process();
        //extern void tts_wink_once(int type)
        //tts_wink_once(1);
        //robot_action_wink_play(0x43,240,300);
        //robot_action_wink_play(0x53,300,400);
        robot_head_use_set(1);
        break;
    case 'c':
        //robot_ai_aiui_debug = 0;
        //DBG_PRINT("robot_ai_aiui_debug(reset) = ",robot_ai_aiui_debug);
        //robot_key_buskey_read();
        //robot_smile_send(1);
        break;
    case 'd':
        //robot_ai_aiui_debug++;
        //if(robot_ai_aiui_debug>3)
        //    robot_ai_aiui_debug = 0;
        //DBG_PRINT("robot_ai_aiui_debug(add) = ",robot_ai_aiui_debug);
        //robot_smile_send(2);
        break;
    case 'e':
        //robot_vcn_idx++;
        //if((robot_vcn_idx<1)||(robot_vcn_idx>4))
        //    robot_vcn_idx = 1;
        //DBG_PRINT("robot_vcn_idx = ",robot_vcn_idx);
        //aiui_vcn_setidx(robot_vcn_idx);
        tts_speak_flag = 0;
        robot_key_use = 0;
        pcm_play_start();
        break;
   // case 'e':
   //     robot_ai_aiui_debug = 2;
  //      break;
    case 'f':
        //app_net_music_test();
        //robot_action_face_auto_once(0,1);
        //robot_action_face_auto_once(0,1);
        //pcm_play_print();
        robot_action_ts15_posi_read();
        break;
    case 'g':
        robot_action_face_auto_once(1,1);
        break;
    case 'h':
        robot_action_face_auto_once(2,1);
        break;
    case 'i':
        robot_action_wink_play(0x33,12,0xa0);
        break;
    case 'j':
        robot_action_wink_play(0x33,14,0xc0);
        break;
    case 'k':
        robot_action_wink_play(0x33,16,0xe0);
        break;
    case 'l':
        robot_action_wink_play(0x33,18,0x100);
        break;
    case 'm':
        robot_action_wink_play(0x33,20,0x120);
        break;
    case 'n':
        //aiui_remind_test();
        //extern time_t iso8601_to_time_t(const char* datetime_str);
        //extern uint32_t iso8601_to_timestamp(const char* str);
        //iso8601_to_timestamp("2025-03-26T15:00:00");
        //DBGU32_PRINT("this_time = ",time(NULL));
        //ifly_aiui_schedule_send();
        //ifly_aiui_schedule_send(NULL);
        break;
    case 'o':
        //ifly_aiui_schedule_send("去喝水");
        //robot_action_face_eye_rand(0);
        break;
    case 'p':
        //robot_action_face_eye_rand(1);
        ts15ctl_settimes(40);
        break;
    case 'q':
        ts15ctl_debug_do();
        //robot_action_face_eye_rand(2);
        break;
    case 'r':
        extern void tbz_speak_set_do(int a);
        tbz_speak_set_do(10);
        break;
    case 's':
        extern void tbz_speak_set_do(int a);
        tbz_speak_set_do(1);
        break;
    case 't':
        mouth_debug_mode = 1 - mouth_debug_mode;
        DBG_PRINT("mouth_debug_mode = ",mouth_debug_mode);
        break;
    case 'u':
        mouth_debug_next = 1 - mouth_debug_next;
        DBG_PRINT("mouth_debug_next = ",mouth_debug_next);
        break;
    case 'v':
        extern int speak_face_flag;
        speak_face_flag = 1 - speak_face_flag;
        DBG_PRINT("speak_face_flag = ",speak_face_flag);
        break;
    case 'w':
        extern int speak_mouth_flag;
        speak_mouth_flag = 1 - speak_mouth_flag;
        DBG_PRINT("speak_mouth_flag = ",speak_mouth_flag);
        break;
    case 'x':
        audio_test_idx++;
        if(audio_test_idx>8)
            audio_test_idx = 1;
            DBG_PRINT("audio_test_idx = ",audio_test_idx);
        //face_emoji_set(0);
        break;
    case 'y':
        robot_control_audio_play_send(audio_test_idx);
        break;
    case 'z':
        break;

    case 'A':
        handle_neck_debug();
        break;

#if 0
    case 'A':
        robot_head_use_set(0);
        robot_uart_bus_debug_set(2);
        break;
    case 'B':
        robot_smile_send(1);//小笑
        break;
    case 'C':
        robot_smile_send(2);//大笑
        break;
    case 'D':
        robot_blink_send(1);
        break;
    case 'E':
        robot_blink_send(2);
        break;
    case 'F':
        robot_blink_send(3);
        break;
    case 'G':
        robot_rub_send(0,2,0);
        break;
    case 'H':
        robot_rub_send(1,2,0);
        break;
#endif
    /*
    case 'A':
        tts_speak_debug = 1 - tts_speak_debug;
        DBG_PRINT("tts_speak_debug = ",tts_speak_debug);
        break;
    case 'B':
        tts_speak_debug_idx = 0;
        DBG_PRINT("tts_speak_debug_idx = ",tts_speak_debug_idx);
        break;
    case 'C':
        tts_speak_debug_idx++;
        DBG_PRINT("tts_speak_debug_idx = ",tts_speak_debug_idx);
        break;
    case 'D':
        extern void tbz_speak_set_do(int a);
        tbz_speak_set_do(1);
        break;
    */
    /*
    case 'A':
        robot_action_th01_speak(10,3,2,10);
        break;
    case 'B':
        robot_action_th01_speak(8,3,2,8);
        break;
    case 'C':
        robot_action_th01_speak(6,3,2,8);
        break;
    case 'D':
        robot_action_th01_speak(5,1,1,10);
        break;
    case 'E':
        robot_action_th01_speak(4,1,1,8);
        break;
    case 'F':
        robot_action_th01_speak(3,1,1,8);
        break;
    case 'G':
        robot_action_th01_speak(6,1,1,8);
        break;
    case 'H':
        robot_action_th01_speak(4,1,1,8);
        break;
    */
#if 0
    case 'A':
        robot_app_debug_mode_switch();
        break;
    case 'I':
        dingflag = 0;
        DBG_PRINT("dingflag = ",dingflag);
        break;
    case 'J':
        dingflag = 1;
        DBG_PRINT("dingflag = ",dingflag);
        break;
    case 'K':
        robot_action_neck_flag = 0;
        DBG_PRINT("robot_action_neck_flag = ",robot_action_neck_flag);
        break;
    case 'L':
        robot_action_neck_flag = 1;
        DBG_PRINT("robot_action_neck_flag = ",robot_action_neck_flag);
        break;
    case 'U':
        tts_speak_flag = 0;
        DBG_PRINT("speak_flag = ",tts_speak_flag);
        break;
    case 'V':
        //wink_data_range_set(3,0,0);
        tts_speak_flag = 1;
        extern void tbz_speak_set_clr();
        extern void tbz_speak_tone_clr();
        tbz_speak_set_clr();
        tbz_speak_tone_clr();
        DBG_PRINT("speak_flag = ",tts_speak_flag);
        break;
    case 'X':
        ws_asr_use = 1;
        DBG_PRINT("ws_asr_use = ",ws_asr_use);
        break;
    case 'Y':
        ws_asr_use = 0;
        DBG_PRINT("ws_asr_use = ",ws_asr_use);
        break;
    //case 'a':
    //    robot_ai_wificfg_setmode();
    //    break;
    //case 'b':
    //    robot_ai_wificfg_tbz();
    //    break;
    //case 'c':
    //    sound_unmute_set(1);
    //    robot_ai_listen();
    //    break;
#endif

#if 0
        // a : posi test
    case 'a':robot_posi_data_once(1,0);break;
        // b c d n : key test
    case 'b':robot_key_data_show();break;
    case 'c':robot_key_switch_set(1);break;
    case 'd':robot_key_switch_set(0);break;
    case 'e':robot_key_get_status(touch_status);break;
        // e f g : irbos test
    case 'f':robot_irobs_switch_set(1);break;
    case 'g':robot_irobs_switch_set(0);break;
    case 'h':robot_irobs_test_once();break;
    case 'i':robot_irobs_test();break;
        // h i : irfollow test
    //case 'h':robot_irfollow_switch_set(1);break;
    //case 'i':robot_irfollow_switch_set(0);break;
        // j k l m : storage test
    case 'j':robot_storage_test_seedroll();break;
    case 'k':robot_storage_test_write();break;
    case 'l':robot_storage_test_read();break;
        // m : action test
    case 'm':robot_action_system_group_play(ROBOT_ACTION_SYSTEM_OPEN);break;
        // o p q r s t u: 2p4g
    case 'o':robot_2p4g_switch_set(0);break;
    case 'p':robot_2p4g_switch_set(1);break;
    case 'q':robot_groupshow_MainMode_Set(0);break;
    case 'r':robot_groupshow_MainMode_Set(1);break;
    case 's':robot_groupshow_SubMode_Set(0);break;
    case 't':robot_groupshow_SubMode_Set(1);break;
    case 'u':robot_2p4g_TestData_Send(1);break;
    case 'v':robot_2p4g_TestData_Send(2);break;
    case 'w':DBG_PRINT("RANDOM TEST >>",robot_random(100));break;
		//x y: iic touch
	//case 'x':robot_ttp_i2c_init();break;
	//case 'y':break;
        // z : vbat
    case 'z':
            DBG_PRINT("robot_vbat_raw_data = ",robot_vbat_raw_data_get());
            DBG_PRINT("robot_vbat_app_level = ",robot_vbat_app_level_get());
            DBG_PRINT("robot_vbat_low_flag = ", Robot_vBatLow_Flag_Get());
            break;
        // asr A-I
    #if 0
    case 'A':robot_asr_switch_set(0);break;
    case 'B':robot_asr_switch_set(1);break;
    case 'C':robot_asr_set_init();break;
    case 'D':robot_asr_get_ready();break;
    case 'E':robot_asr_function_set(ROBOT_ASR_FINDNOISE);break;
    case 'F':robot_asr_function_set(ROBOT_ASR_ADJUSTNOISE);break;
    case 'G':robot_asr_set_recog_once();robot_asr_word_make();break;
    case 'H':robot_asr_set_recog_break();break;
    case 'I':robot_asr_function_set(ROBOT_ASR_RECORD);break;
    case 'J':robot_asr_word_make();break;
    #endif
    case 'A':wanson_asr_lock_debug();break;
        // irfollow: I-L
    #if 0
    case 'K':robot_irfollow_switch_set(0);break;
    case 'L':robot_irfollow_switch_set(1);break;
    case 'M':robot_irfollow_switch_set(2);break;
    case 'N':robot_irfollow_switch_set(4);break;
    #endif
    case 'O':robot_main_breathe_set(0);break;
    case 'P':robot_main_breathe_set(1);break;
        // app debug mode X Y
    //case 'X':robot_app_debug_mode_set(0);break;
    //case 'Q':robot_app_debug_mode_set(1);break;

    case 'R':robot_action_ts15_alllock(0);break;
    case 'S':robot_action_data_saveplay(2,1);break;
    case 'T':robot_action_ts15_posi_read();break;
    case 'U':robot_action_adjusting_compare(1);break;
    case 'V':robot_action_adjusting_load();break;

    //case 'W':read_gyroData();break;
    //case 'Z':robot_program_read_distance();break;
    //case 'Q':light_ctl_set_col(1, 2);break;
    case 'X':
        Tbz_Config_Save(6,1);
        break;
    case 'x':
        light_ctl_set_col(2,10);
        break;
    case 'Y':groupshow_test(0);
        break;
    case 'y':robot_program_get_robot_posi();
        break;
    case 'Z':
        robot_posi_data_once(1,3);
        break;
    default :
        break;
#endif
    }
}

int robot_get_int(char *num,int sz)
{
    int i;
    int ret = 0;
    for(i=0;i<sz;i++)
    {
        ret = ret*10+(num[i]-'0');
    }
    return ret;
}

void robot_main_inputGo_MOVE(u8* data,int sz)
{
    int i,j;
    for(i=0;i<sz;i++)
    {
        switch(data[i])
        {
        case 'F':robot_action_cmd_play(1,1);break;
        case 'B':robot_action_cmd_play(2,1);break;
        case 'L':robot_action_cmd_play(3,1);break;
        case 'R':robot_action_cmd_play(4,1);break;
        case 'S':robot_action_cmd_play(3,15);break;
        case 'T':robot_action_cmd_play(4,15);break;
        case 'X':
        case 'x':
            j = robot_get_int(&data[i+1],3);
            if(data[i]=='x')
                robot_audio_cmd_play(j,0);
            else
                robot_audio_cmd_play(j,1);
            i+=3;
            break;
        case 'Y':
        case 'y':
            j = robot_get_int(&data[i+1],3);
            robot_emoji_cmd_play(j);
            i+=3;
            break;
        case 'H':
            //help
            STR_PRINT("HELP : \r\n");
            STR_PRINT("F:forward, B:back, L:left, R:right\r\n");
            STR_PRINT("S:Left Turn T:Right Turn\r\n");
            STR_PRINT("X:audio wait x:audio no wait\r\n");
            STR_PRINT("audio: 0 break, 1-20 voice 21-30, music 31-90 tone\r\n");
            STR_PRINT("Y:emoji\r\n");
            STR_PRINT("Z:Exit\r\n");
            break;
        case 'Z':
            robot_main_input_mode_set(MAININPUT_MODE_CMD);
            break;break;
        }
    }
    robot_action_cmd_play(0,0);
}

void robot_main_inputGo_MOTOR(u8* data,int sz)
{
    int i,j=0;
    for(i=0;i+ROBOT_MOTOR_CMD_SZ<=sz;i+=ROBOT_MOTOR_CMD_SZ)
    {
        j = robot_motor_cmdGo(data+i);
        if(j!=1)
            break;
    }
    DBG_PRINT("end_flag = ",j);
    if(j!=2)
        robot_motor_stop();
}

void robot_main_hello()
{
    DBG_PRINT("Hello World TH01_V0.2 20241225",0);
    //RF2P4G_SEND("Uart GS Test",12);
    //BUS_WRITE("Uart BUS Test",13);
    //ex_audio_play("hello.mp3",NULL);
    //extern int auto_action_step;
    //auto_action_step = 1;
}

void robot_main_status()
{

}

#if 0
void robot_main_inputGo_CMD(int cmd)
{
    switch(cmd)
    {
    case '0':robot_main_hello();break;
    case '1':robot_main_status();break;
    case '2':break;
    case '3':robot_main_input_mode_set(MAININPUT_MODE_HEAD);break;
    case '4':robot_main_input_mode_set(MAININPUT_MODE_CONFIG);break;
    case '5':robot_main_input_mode_set(MAININPUT_MODE_SSID_PWD);break;
    case '6':robot_main_input_mode_set(MAININPUT_MODE_UARTBUS);break;
    case '7':robot_main_input_mode_set(MAININPUT_MODE_RF2P4G);break;
    case '8':robot_main_input_mode_set(MAININPUT_MODE_APP);break;
    case '9':robot_main_input_mode_set(MAININPUT_MODE_DATA);break;
    default:
        robot_main_inputGo_TEST(cmd);
        break;
    }
}
#else
void robot_main_inputGo_CMD(char cmd[],int len)
{
    int i;
    for(i=0;i<len;i++)
    {
        if((cmd[i]==0xFF)||(cmd[i]==-1))
        {
            len = i;
            break;
        }
    }
    cmd[len] = '\0';
    robot_command_make(cmd);
}
#endif

int robot_main_judge_cmd_exit(u8* data,int sz)
{
    const u8 end_code[] ="ENDENDEND";//0x45,0x4E,0x44
    const int ec_sz = 9;
    int i;

    if(sz!=ec_sz)
        return 0;
    for(i=0;i<ec_sz;i++)
    {
        if(data[i]!=end_code[i])
            return 0;
    }
    return 1;
}

int robot_main_judge_head_bus_cmd(u8* data,int sz)
{
    if(sz<3)
        return 0;
    return ((data[0]==0xF5)&&(data[1]==0x5F)&&(data[2]==0xFE));
}

void robot_main_inputGo_DATA(u8* data,int sz)
{
    DBG_PRINT("robot_main_inputGo_DATA sz = ",sz);
    CTRL_SEND(data,sz);
}

void robot_main_inputGo_APP(u8* data,int sz)
{
    DBG_PRINT("robot_main_inputGo_APP sz = ",sz);
    if(robot_main_judge_cmd_exit(data,sz))
    {
        robot_2p4g_switch_set(0);
        robot_main_input_mode_set(MAININPUT_MODE_CMD);
    }
    else
    {
        robot_app_entrance(data,sz);
    }
}


void robot_main_inputGo_RF2P4G(u8* data,int sz)
{
    int i;
    DBG_PRINT("robot_main_inputGo_RF2P4G sz = ",sz);
    if(robot_main_judge_cmd_exit(data,sz))
    {
        robot_2p4g_switch_set(0);
        robot_main_input_mode_set(MAININPUT_MODE_CMD);
    }
    else
    {
        RF2P4G_WRITE(data,sz);
    }
}

void robot_main_inputGo_CFG(u8* data,int sz)
{
    int i;
    int end = 0;
    DBG_PRINT("robot_main_inputGo_CFG sz = ",sz);
    if((data[0]!=0xfc)||(data[1]!=0xcf))
    {
        DBG2_PRINT("format err [fc] [cf] [cfg1_idx] [cfg1_data] ..",data[0],data[1]);
        return ;
    }
    for(i=2;i<sz;i+=2)
    {
        if(data[i]==0xff)
        {
            if(data[i+1]==0xff)
                end = 1;
        }
        else
        {
            Tbz_Config_Save(data[i+0],data[i+1]);
        }
    }
    Tbz_Config_Read(1);
    if(end)
    {
        robot_main_input_mode_set(MAININPUT_MODE_CMD);
    }
}
//int32_t robot_bus_command_makerobot_bus_command_make(int32_t id,int32_t cmd,int32_t add,int32_t paralength,uint8_t *para,uint8_t output[])
int robot_main_inputGo_BUSCMD(u8* data,int sz)
{
    uint8_t comm[256];
    int csz;
    if(sz<4)
    {
        return -1;
    }
    if(sz!=data[3]+4)
    {
        return -2;
    }
    csz = robot_bus_command_make(data[0],data[1],data[2],data[3],&data[4],comm);
    BUS_WRITE(comm,csz);
    return 0;
}

void robot_main_inputGo_UARTBUS(u8* data,int sz)
{
    if(robot_main_judge_cmd_exit(data,sz))
    {
        robot_main_input_mode_set(MAININPUT_MODE_CMD);
    }
    else
    {
        if(robot_main_judge_head_bus_cmd(data,sz))
        {
            if(robot_main_inputGo_BUSCMD(data+3,sz-3)==0)
            {
                STR_PRINT("BUS CMD SEND OK\r\n");
            }
            else
            {
                STR_PRINT("BUS CMD SEND ERR: FORMAT ERR\r\n");
            }
        }
        else
        {
            BUS_WRITE(data,sz);
            STR_PRINT("BUS DATA SEND OK\r\n");
        }

    }
    return ;
}

void robot_main_inputGO_WIFI(u8* data,int sz)
{
    static bool is_waiting_for_credentials = true;
    DBG_PRINT("robot_main_inputGO_WIFI sz = ", sz);

    if(is_waiting_for_credentials)
    {
        wife_ssid_pwd_input(data, sz);
        is_waiting_for_credentials = false;
    }
    else
    {
        is_waiting_for_credentials = true;
        STR_PRINT("Please enter your SSID and password separated by '#'\r\n");
    }
}

int robot_hex2dec(int hex)
{
    return (hex/16)*10+(hex%16);
}

void robot_mian_inputGO_Servo(u8 *data,int sz)
{
    #if 1
    int i;
    //memset(servo_data,0,sizeof(servo_data));
    int code = robot_main__getCMD(data,sz);
    for(i=2;i<sz;i++)
    {
        data[i] = robot_hex2dec(data[i]);
    }
    switch(code)
    {
    case 0://彻底松开
        //robot_action_th01_mode_write(0);
        break;
    case 1://中位
        //robot_action_th01_mode_write(1);
        break;
    case 2://解锁
        //robot_action_th01_mode_write(2);
        break;
    case 3://上锁
        //robot_action_th01_mode_write(3);
        break;
    case 4://数据写入（左）
        //robot_action_th01_data_write(1,data+2,sz-2);
        break;
    case 5://数据写入（右）
        //robot_action_th01_data_write(2,data+2,sz-2);
        break;
    case 6://数据写入（顺）
        //robot_action_th01_data_write(3,data+2,sz-2);
        break;
    case 7://数据修改（单个）
        //robot_action_th01_data_write_one(data[2],data[3],data[4]*100+data[5]);
        break;
    case 8:
        break;
    case 9:
        break;
    case 10://嘴巴接口
        break;
    case 11://单舵机控制，组合舵机控制
        break;
    case 12://眨眼调试接口
        /*
            眨眼调试接口
            类型[11/12/13 左眼，打开/闭上/眨眼; 21/22/23 右眼，打开/闭上/眨眼; 31/32/33 双眼，打开/闭上/眨眼]
            眨眼时间(ms)，停留时间（10ms），
            眼帘幅度（主），眉毛幅度（单眼），眼球幅度（双眼）
            FC CF 33 01 06 03 00 00 双眼眨一下，眨动时间1ms，停留时间60ms，眨眼眼帘幅度3，眉毛眼球不动。
        */
        robot_action_wink_play_debug_enter(data+2,sz-2);
        break;
    case 13://脖子调试接口
        /*
            脖子调试
            方向[0归位，1左，2右] 幅度 运动时间(10ms) 停留时间(10ms)
            FD DF 01 05 01 02 脖子向左运动5度，时间为10ms，停留时间为10ms
        */
        robot_action_neck_debug_make(data+2,sz-2);
        break;
    case 14://显示数据
        break;
    case 15:
          /*
        组合调试
        类型[0:]
        FF FF
        */
        robot_action_th01_ts_steps_set2(data+2,sz-2);
        break;
        break;
    default:
        break;
    }
    //if((code>=0)&&(code<=7))
    //{
    //    robot_action_th01_data_send();
    //}
    #endif
}

void robot_mian_inputGO_Head(u8 *data,int sz)
{
    data[sz] = '\0';
    robot_head_command_send(data);
}

void robot_main_UARTBUS_read_once()
{
    u8 ReadBuffer[128];
    u8 para[128];
    u16 type;
    u16 id,err,plen;
    int readsz;
    readsz = BUS_READ(ReadBuffer,127);

    if(readsz>0)
    {
        DBG_PRINT("UART BUS DATA READ Size = ",readsz);
        HEXS_PRINT("UART BUS DATA",ReadBuffer,readsz);
        if(type = robot_bus_data_reply(ReadBuffer,readsz,&id,&err,&plen,para))
        {
            STR_PRINT("UART BUS REPLY : \r\n");
            DBG_PRINT("ID : ",id);
            DBG_PRINT("TYPE : ",type);
            DBG_PRINT("ERR : ",err);
            if(plen == 2)
            {
                u16 data16 = (para[0]<<8)+(para[1]);
                UDBG_PRINT("DATA16 : ",data16);
            }
            else
            {
                if(plen)
                    HEXS_PRINT("DATA:",para,plen);
            }
        }
    }
}


#define ROBOT_READ_SIZE 128

static void robot_main_input()
{
    static u8 UartBuffer[ROBOT_READ_SIZE];
    int UartReadSz;
    #if 0
    u32 ms_start,ms_now;
    int sz;
    //DBG_PRINT("robot_main_input",0);
    if(__robot_main->uart_dev)
        __robot_main->data_length = dev_read(__robot_main->uart_dev, UartBuffer, TBZ_RX_SIZE);
    else
        __robot_main->data_length = 0;
    //DBG_PRINT("robot_main_input length = " ,__robot_main->data_length);
    if(__robot_main->data_length <= 0)
    {
        //DBG_PRINT("input nothing",__robot_main->data_length);
        return;
    }

    #if 1
    while(1)
    {
        if(__robot_main->uart_dev)
            sz = dev_read(__robot_main->uart_dev, UartBuffer+__robot_main->data_length, TBZ_RX_SIZE-__robot_main->data_length);
        else
            sz = 0;
        if(sz<=0)
            break;
        __robot_main->data_length += sz;
    }
    #endif
    #else
    UartReadSz = robot_uart_main_read(UartBuffer,ROBOT_READ_SIZE);
    if(UartReadSz<=0)
        return ;
    #endif

    switch(RobotMainInputMode)
    {
        case MAININPUT_MODE_CMD :
            robot_main_inputGo_CMD(UartBuffer,UartReadSz);
            break;
        case MAININPUT_MODE_DATA :
            robot_main_inputGo_DATA(UartBuffer,UartReadSz);
            break;
        case MAININPUT_MODE_APP :
            robot_main_inputGo_APP(UartBuffer,UartReadSz);
            break;
        case MAININPUT_MODE_CONFIG :
            robot_main_inputGo_CFG(UartBuffer,UartReadSz);
            break;
        case MAININPUT_MODE_RF2P4G:
            robot_main_inputGo_RF2P4G(UartBuffer,UartReadSz);
            break;
        case MAININPUT_MODE_UARTBUS:
            robot_main_inputGo_UARTBUS(UartBuffer,UartReadSz);
            break;
        case MAININPUT_MODE_MOTOR:
            robot_main_inputGo_MOTOR(UartBuffer,UartReadSz);
            break;
        case MAININPUT_MODE_SSID_PWD :
            robot_main_inputGO_WIFI(UartBuffer,UartReadSz);
            break;
        case MAININPUT_MODE_SERVO:
            robot_mian_inputGO_Servo(UartBuffer,UartReadSz);
            break;
        //case MAININPUT_MODE_HEAD:
        //    robot_mian_inputGO_Head(UartBuffer,UartReadSz);
        //    break;
        default:
            break;
    }

}




/*
TS15回码：
01，过压保护，第零位
04，过温保护，第一位
40，堵转保护，第五位。
*/
int robot_bus_ts15ack_read(uint16_t data[][2],int sz,int timeout)
{
    int i;
    int k;
    u8 buff[40];
    int bsz;
    int rsz = 0;
    u32 curtime = ROBOT_GETTIME_MS();
#if 0
    static u8 s_remain_byte = 0;
    static bool s_has_remain = false;
#endif
    while(rsz < sz)
    {
        u32 nowtime = ROBOT_GETTIME_MS();
        if(nowtime - curtime > (u32)timeout) {
            //DBG2_PRINT("[TS15ACK] Timeout! Elapsed Remaining", timeout, sz-rsz);
            break;
        }


        if((bsz = robot_bus_data_read(buff,40)) < 0) {
            if(bsz == -2) {
                ROBOT_DELAY_US(100);
                continue;
            }
            break;
        }

        //if(robot_bus_uart_flag)
        if(0)
        {
            DBG_PRINT("BSZ = ",bsz);
            if(bsz>0)
                HEXS_PRINT("BSZ DATA ",buff,bsz);
        }

#if 1
        for(i=0;i<bsz;i++)
        {
            if((buff[i]>=1)&&(buff[i]<=17))
            {
                int id = buff[i];
                if(data[id][0] == 0)
                {
                    data[id][0] = 1;
                    rsz++;
                }
            }
        }
#else
        i = 0;
        if(s_has_remain){
            if(bsz >=1){
                uint8_t id = s_remain_byte;
                uint8_t status = buff[0];
                if(status == 0x00 && id>=1 && id<=17){
                    if(data[id][0] == 0){
                        data[id][0] = 1;
                        rsz++;
                    }
                }
                i = 1;
            }
            s_has_remain = false;
        }


        for(; i<bsz; ){
            if(i+1 >= bsz){
                s_remain_byte = buff[i];
                s_has_remain = true;
                break;
            }

            uint8_t id = buff[i];
            uint8_t status = buff[i+1];

            if(id < 1 || id > 17) {
                i += 2;
                continue;
            }

            if(status != 0x00) {
                data[id][0] = 2;
                rsz++;
            }

            /* 有效响应 */
            if(data[id][0] == 0){
                data[id][0] = 1;
                rsz++;
            }
            i += 2;
        }
#endif // 1
    }

#if 0
    if(s_has_remain){
        s_has_remain = false;
    }
#endif
    return rsz;
}

int robot_bus_read_idle()//检查总线线路是否空闲
{
    return robot_bus_ts15_idle();

}
#if 0
void robot_bus_data_clr()
{
    u8 ClrBuffer[32];
    //DBG_PRINT("robot_bus_data_clr ",__robot_bus);
    if(__robot_bus->uart_dev)
    {
        do
        {
            __robot_bus->data_length = dev_read(__robot_bus->uart_dev, ClrBuffer, 31);
        }while(!__robot_bus->data_length);
    }
    else
    {
        __robot_bus->data_length = 0;
    }
}
#endif

void robot_bus_data_clr_once()
{
    //if(!robot_bus_read_idle())
    //    return ;
    //robot_bus_data_clr();
}

void robot_main_breathe_set(uint16_t cs)
{
    robot_main_breathe_cs = cs;
    DBG_PRINT("robot_main_breathe_cs = ",cs);
}

int action_debug_cnt = 15;

void robot_main_breathe()
{
    static int breathe_count = 0;
    static int last_time = 0;
    int now_time = ROBOT_GETTIME_MS();
    if(!robot_main_breathe_cs)
        return ;

    if((last_time==0)||(now_time-last_time>10000))//10sec
    {
        DBG_PRINTF("ROBOT_BREATHE : count = %d time = %d sec",breathe_count,now_time/1000);
        breathe_count++;
        action_debug_cnt++;
        last_time = now_time;
    }

}

void robot_function_once()
{
    //robot_vbat_once();

    //robot_irobs_data_show();
    //robot_key_test_once();
    //robot_irfollow_move_go();
    //robot_remote_keep_once();
    //robot_2p4g_once();
//    robot_asr_set_recog_once();
#if 0
    if(robot_key_use)
    {
        robot_key_buskey_read();
        robot_key_data_make();
    }
#endif
#if 0

    asr_data_make();
    asr_aciton_end();
#endif
}

/*
void robot_main_heartbeat()
{
    static u32 last_timer = 0;
    static int time = 0;
    u32 cur = ROBOT_GETTIME_MS();
    if(cur-last_timer>=1000)
    {
        last_timer = cur;
        DBG_PRINT("lumos",time);
        time++;
        wdt_clear();
    }
}
*/


static void robot_help_task(void *priv)
{
    //DBG_PRINT("robot_help_task",0);
    while(1)
    {
        //DBG_PRINT("robot_help_task",1);
        //if(robot_key_use)
        //    robot_key_timer_once();
        //robot_main_breathe();
        //uart2p45_setpin_blink();
        //uart2p4g_heartbeat();
        //robot_main_heartbeat();
        ROBOT_DELAY_MS(5);
    }
}


int robot_main_aiui_action = 0;
extern int tbz_next_asr_flag;

void robot_main_aiui_action_set(int action)
{
    DBG_PRINT("robot_main_aiui_action_set ",action);
    robot_main_aiui_action = action;
}

void robot_main_aiui_action_do()
{

    if((robot_main_aiui_action)&&(tbz_next_asr_flag==5))
    {
        switch(robot_main_aiui_action)
        {
        case 1:
            DBG_PRINT("CRY!!",0);
            break;
        case 2:
            DBG_PRINT("SMILE!!",0);
            break;
        case 3:
            DBG_PRINT("SLEEP!!",0);
            robot_control_audio_play_send(9);
            break;
        }
        if(robot_main_aiui_action!=3)
        {
            msleep(2000);
            DBG_PRINT("tbz_next_asr_flag = ",tbz_next_asr_flag);
            //tbz_next_asr_flag = 1;
            parseIvwResultTest(0);
        }
        robot_main_aiui_action = 0;
    }

    /*
    1.语音识别处于开始状态
    2.语音播放结束
    */
    if(asr_nodingstep1 && asr_nodingstep2)
    {
        asr_nodingstep1 = 0;
        asr_nodingstep2 = 0;
        robot_main_debug_output("start_recognize(asr_nodingstep)",2);
        ifly_aiui_start_recognize_process();
    }

    if(robot_ai_aiui_reset_set)
    {
        u32 now = timer_get_ms();
        if(now - robot_ai_aiui_reset_time>1500)
        {
            robot_ai_aiui_reset_set = 0;
            robot_ai_aiui_reset_time = 0;
            //添加没听清提示
            DBG_PRINT("ASR Err, Auto Reset",0);
            //tbz_next_asr_flag = 1;
            parseIvwResultTest(0);
        }
    }
}



void tts_speak_mouth()
{
    int r = RANDOM(8);
    if(mouth_debug_mode)
    {
        if(mouth_debug_next)
        {
            mouth_debug_idx++;
            if(mouth_debug_idx>=8)
                mouth_debug_idx = 0;
            mouth_debug_next = 0;
        }
        r = mouth_debug_idx;
        DBG_PRINT("tts_speak_mouth = ",mouth_debug_idx);
    }
    #if 1
    switch(r)
    {
    case 0:
        robot_action_th01_speak(5,1,1,4);
        break;
    case 1:
        robot_action_th01_speak(6,1,1,4);
        break;
    case 2:
        robot_action_th01_speak(7,1,1,4);
        break;
    case 3:
        robot_action_th01_speak(8,1,1,4);
        break;
    case 4:
        robot_action_th01_speak(5,1,2,4);
        break;
    case 5:
        robot_action_th01_speak(6,2,3,4);
        break;
    case 6:
        robot_action_th01_speak(8,2,3,4);
        break;
    case 7:
        robot_action_th01_speak(9,3,2,4);
        break;
    }
    #else
    switch(r)
    {
    case 0:
        robot_action_th01_speak(8,4,1,4);
        break;
    case 1:
        robot_action_th01_speak(10,2,1,4);
        break;
    case 2:
        robot_action_th01_speak(12,2,1,4);
        break;
    case 3:
        robot_action_th01_speak(6,3,2,4);
        break;
    case 4:
        robot_action_th01_speak(9,3,2,4);
        break;
    case 5:
        robot_action_th01_speak(5,1,1,4);
        break;
    case 6:
        robot_action_th01_speak(6,1,1,4);
        break;
    case 7:
        robot_action_th01_speak(7,1,1,4);
        break;
    }
    #endif
}

u32 gol_rec_ms = 0;

void robot_main_rec_show(int flag)
{
    static u32 rec_idx = 0;
    if(flag==0)
    {
        rec_idx = timer_get_ms();
        robot_main_debug_output_uint32("rec reset",rec_idx);
    }
    else
    {
        gol_rec_ms = timer_get_ms()-rec_idx;
        robot_main_debug_output_uint32("rec..",gol_rec_ms);
    }
}

int tbz_speak_set = 0;
int tone_music_flag = 0;
int dingflag = 0;

void tbz_speak_set_do(int a)
{
    DBG_PRINT("tbz_speak_set_do",a);
    if(tts_speak_flag)
        tbz_speak_set = a;
}

void tbz_speak_set_clr()
{
    tbz_speak_set = 0;
}

void tbz_speak_tone_do()
{
    if(tts_speak_flag)
        tone_music_flag = 1;
}

void tbz_speak_tone_clr()
{
    tone_music_flag = 0;
}

void tts_wink_once(int type)
{
    if(speak_wink_flag==0)
        return ;
    switch(type)
    {
    case 1:
        robot_action_wink_play(0x13,40,0x50);
        break;
    case 2:
        robot_action_wink_play(0x23,40,0x50);
        break;
    case 3:
        robot_action_wink_play(0x33,40,0x50);
        break;
    case 4:
        robot_action_wink_play(0x43,300,400);
        break;
    case 5:
        robot_action_wink_play(0x53,300,400);
        break;
    }
}
void tts_speak_once()
{
    static int time = 0;
    static u32 next_wink = 0;
    int r;
    u32 now_time = 0;
    static int havespeak = 0;

    extern int tbz_next_asr_flag;
    extern int ifly_music_flag;
    extern int aiui_play_mp3_flag;
    extern int robot_key_tone_flag;

    now_time = ROBOT_GETTIME_MS();

    //if((tbz_next_asr_flag==2)||(tbz_next_asr_flag==3)||(tbz_next_asr_flag==4))

    if(tone_music_flag)
    {
        int tone_status = os_get_tone_status();
        if(tone_status)
        {
            tone_music_flag = 2;
        }
        else
        {
            if(tone_music_flag==2)
                tone_music_flag = 0;
        }
        DBG2_PRINT("tone flag&status = ",tone_music_flag,tone_status);
    }
    if(tbz_next_asr_flag)
        DBG_PRINT("tbz_next_asr_flag = ",tbz_next_asr_flag);
    if(tbz_speak_set)
        DBG_PRINT("tbz_speak_set = ",tbz_speak_set);
    if(ifly_music_flag)
        DBG_PRINT("ifly_music_flag = ",ifly_music_flag);
    if(tone_music_flag)
        DBG_PRINT("tone_music_flag = ",tone_music_flag);
    if(aiui_play_mp3_flag)
        DBG_PRINT("aiui_play_mp3_flag = ",aiui_play_mp3_flag);
    if((tbz_next_asr_flag>1)||(tbz_speak_set)||(ifly_music_flag)||(tone_music_flag)||aiui_play_mp3_flag)
    {
        //DBG_PRINT("TTS PLAY step = ",time>0);
        //robot_action_th01_speak(10,1,1,8);
        tts_speak_mouth();
        robot_key_ban_set(5000);
        if(robot_key_tone_flag)
        {
            if(robot_key_tone_flag==1)
                havespeak = 4;
            else
                havespeak = 5;
        }
        else if((tbz_next_asr_flag>1)||(aiui_play_mp3_flag))
        {
            //havespeak = 3;
            havespeak = 1;
        }
        else
        {
            if(havespeak==0)
                havespeak = 1;
        }
        DBG_PRINT("havespeak = ",havespeak);
        if(tbz_speak_set>0)
        {
            tbz_speak_set--;
        }
    }
    else
    {
        if(havespeak)
        {
            DBG_PRINT("have done",havespeak);
            robot_action_face_eye_rand(2);//说完话眼睛要恢复。
            if(havespeak>=2)
            {
                if(havespeak==3)
                    r = 9;
                else if(havespeak==4)
                    r = 10;
                else if(havespeak==5)
                    r = 11;
                else
                    r = RANDOM(8);
                    DBG_PRINT("wink r = ",r);
                if(r == 3)
                {
                    msleep(1500);
                    tts_wink_once(1);
                    robot_key_ban_set(100);
                }
                else if(r==6)
                {
                    msleep(1500);
                    tts_wink_once(2);
                    robot_key_ban_set(100);
                }
                else if(r==9)
                {
                    msleep(500);
                    tts_wink_once(5);
                    robot_key_ban_set(2000);
                }
                else if(r==10)
                {
                    msleep(500);
                    robot_action_wink_play(0x43,300,400);
                    robot_key_ban_set(2000);
                }
                else if(r==11)
                {
                    msleep(500);
                    robot_action_wink_play(0x53,300,400);
                    robot_key_ban_set(2000);
                }
            }
            else
            {
                r = 0;
            }
            if((r==3)||(r==6)||(r==9))
            {
                if(next_wink-now_time<2000)//10s
                    next_wink+=2000;
            }
            robot_key_tone_flag = 0;
            havespeak  = 0;
        }
        time = 0;
    }
    time++;

    if(next_wink==0)
    {
        now_time = ROBOT_GETTIME_MS();
        next_wink = now_time+RANDOM(2000)+3000;
        //next_wink = now_time+RANDOM(4000)+5000;
    }
    else
    {
        if(now_time >= next_wink)
        {
            tts_wink_once(3);
            //robot_key_ban_set(100);
            next_wink = 0;
        }
    }
}

char open_audio[20] = "PowerOn.mp3";
char *get_open_audio()
{
    if(ROBOT_DATA_VCN)
    {
        sprintf(open_audio,"open%d.mp3",ROBOT_DATA_VCN);
    }
    return open_audio;
}

void robot_main_speak_once(int debug)
{
    extern int ifly_music_flag;
    extern int aiui_play_mp3_flag;
    extern int robot_key_tone_flag;
    static int have = 0;
    static int playidx = 1;
    int speak = debug;

    if(ifly_music_flag)
        DBG_PRINT("ifly_music_flag = ",ifly_music_flag);
    if(tone_music_flag)
        DBG_PRINT("tone_music_flag = ",tone_music_flag);
    if(aiui_play_mp3_flag)
        DBG_PRINT("aiui_play_mp3_flag = ",aiui_play_mp3_flag);

    if(speak==-1)
        speak = (os_get_tone_status() || aiui_play_mp3_flag);
    if(speak)
    {
        int speak_open_where = 80+robot_random(15);
        int speak_open_step = 12 + robot_random(4);
        int speak_close_where = -30;
        int speak_close_step = 14 + robot_random(4);
        //int speak_open_time = (140*(speak_open_where+10))/(3*speak_open_step)+25;
        //int speak_close_time = (140*(speak_open_where+10))/(3*speak_close_step)+25;
        DBG2_PRINT("speak_open >> ",speak_open_where,speak_open_step);
        DBG2_PRINT("speak_close >> ",speak_close_where,speak_close_step);
        //DBG2_PRINT("speak_time >>",speak_open_time,speak_close_time);
        robot_speak_send(1,speak_open_step,speak_open_where);
        robot_speak_status_wait(4,25);
        robot_speak_send(2,speak_close_step,speak_close_where);
        robot_speak_status_wait(6,25);
        if(have==0)
        {
            if(robot_key_tone_flag)
            {
                have = 3;
            }
            else if(aiui_play_mp3_flag)
            {
                have = 2;
            }
            else
            {
                have = 1;
            }
        }
        robot_key_ban_set(0);
    }
    else
    {
        if(have)
            DBG_PRINT("have = ",have);
        if(have)
        {

            robot_speak_send(0,0,0);
            #if 0
            if(have>1)
                robot_sleep_send(50);
            if(have==2)
            {
                robot_smile_send(2);
            }
            else if(have==3)
            {
                //int r = robot_random(10);
                robot_key_ban_set(0);
                switch(playidx)
                {
                case 1:robot_smile_send(3);break;
                case 2:robot_blink_send(1,1);break;
                case 3:robot_blink_send(2,1);break;
                }
                playidx++;
                if(playidx>3)
                    playidx = 1;
            }
            #endif
            have = 0;
        }
    }
}



static void robot_main_task(void *priv)
{
    int loop_count = 0;
    robot_main_sem_pend();
    robot_ttp_i2c_init();
    os_set_tone_ban(0);
    DBG_PRINTF("BEGIN:");
    while(1)
    {
        //if(robot_main_breathe_cs)
        //DBG_PRINTF("LOOP : count = %d time = %d",loop_count++,ROBOT_GETTIME_MS());
        robot_main_breathe();
        robot_main_input();
        if(RobotMainInputMode == MAININPUT_MODE_CMD)
            robot_bus_data_clr_once();
        else if(RobotMainInputMode == MAININPUT_MODE_UARTBUS)
            robot_main_UARTBUS_read_once();
        robot_control_once();
        if(get_head_status())
            robot_main_speak_once(-1);

        if(robot_key_use_get())
        {
            robot_ttp_server();
            if(robot_key_use_get()==2)
                robot_key_data_make();
        }

        //if(tts_speak_flag)
        //    tts_speak_once();
        //robot_main_aiui_action_do();
        //extern void robot_main_schedule_loop();
        //robot_main_schedule_loop();

        //if(robot_debug_flag)
        //    DBG_PRINT("debug",444);
        //if(RobotMainInputMode == MAININPUT_MODE_CMD)
        //robot_function_once();
#if 0
        if(action_debug_cnt>=20)
        {
            action_debug_cnt = 0;
            robot_action_data_saveplay(2,10);
        }

        if(auto_action_step)
        {
            switch(auto_action_step)
            {
            case 1:robot_action_data_saveplay(2,10);break;
            case 3:robot_action_data_saveplay(2,11);break;
            case 5:robot_action_data_saveplay(2,12);break;
            }
            auto_action_step++;
            if(auto_action_step>=6)
                auto_action_step = 0;
        }
#endif
        //if(robot_debug_flag)
        //    DBG_PRINT("debug",555);
        wdt_clear();
    }
}



/*
//这个是旧接口，现在已经不需要，归到control的rf2p4g_once里面。
static void robot_groupshow_task(void *priv)
{
    const unsigned char init_tag[5] = {0xFF,0xFF,0xFF,0xFF,0xFF};
    robot_2p4g_data_send(init_tag,5);
    while(__robot_main->run_flag)
    {
        robot_2p4g_groupshow_make();
    }
}
*/
void robot_irfollow_timer(void *priv)
{
    robot_ir_timer_once();//20ms
}

void robot_irobs_timer(void *priv)
{
    robot_irobs_timer_once();//1ms
}

void robot_key_timer(void *priv)
{
    robot_key_timer_once();//20ms
}



void robot_init(void *priv)
{
    robot_uart_init();
    //robot_rf2p4g_uart_init();
    robot_storage_init();
    sound_unmute_set(1);
    //robot_vbat_init();
    //robot_dcin_init();
    //robot_asr_init();
    //robot_config_init();
    //robot_key_init();
    //robot_main_timer_init();

    robot_main_sem_creat();
    //thread_fork("robot_help_task", 29, 2048, 0, 0, robot_help_task, NULL);
    thread_fork("robot_main_task", 30, 2048, 0, 0, robot_main_task, NULL);

}
late_initcall(robot_init);

uint8_t robot_data_checksum_uint8t(uint8_t *data,uint16_t sz)
{
    int i;
    uint8_t cs=0xFF;
    for(i=1;i<sz;i++)
        cs ^= data[i];
    return cs;
}

uint8_t robot_data_checksum_flash(uint8_t *data,uint16_t sz)//for flash data
{
    int i;
    uint8_t cs=0xFF;
    for(i=0;i<sz;i++)
        cs ^= data[i];
    return cs;
}

extern unsigned int random32(int type);

uint16_t robot_random(uint16_t m)
{
    return random32(0)%m;
}



//typedef unsigned int timestamp_t; // 使用unsigned int替代long long

static const int days_in_month[] =
{
    31, 28, 31, 30, 31, 30,
    31, 31, 30, 31, 30, 31
};

static int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static uint32_t calculate_days(int year, int month, int day)
{
    uint32_t days = 0;
    int y;
    int m;

    /* 计算年份天数 */
    for (y = 1970; y < year; y++)
    {
        days += 365 + is_leap_year(y);
    }

    /* 计算月份天数 */
    for (m = 1; m < month; m++)
    {
        days += days_in_month[m-1];
        if (m == 2 && is_leap_year(year))
        {
            days += 1;
        }
    }

    /* 当月天数 */
    days += day - 1;
    return days;
}

uint32_t iso8601_to_timestamp(const char* str)
{
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    int max_day;
    uint32_t total_days;
    uint32_t timestamp;

    /* 解析输入字符串 */
    if (sscanf(str, "%d-%d-%dT%d:%d:%d",
              &year, &month, &day, &hour, &min, &sec) != 6)
    {
        return (uint32_t)0;
    }

    /* 有效性校验 */
    if (year < 1970 || month < 1 || month > 12 || day < 1 ||
        hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59)
    {
        return (uint32_t)0;
    }

    /* 校验当月天数 */
    max_day = days_in_month[month-1];
    if (month == 2 && is_leap_year(year))
    {
        max_day = 29;
    }
    if (day > max_day)
    {
        return (uint32_t)0;
    }

    /* 计算总秒数 */
    total_days = calculate_days(year, month, day);
    timestamp = (uint32_t)(total_days * 86400U +
                             hour * 3600U +
                             min * 60U +
                             sec);
    //timestamp -= 8*3600;//时区修正，咱是东八区，后续要在config中加入一个时区的配置
    DBGU32_PRINT("time >> ",timestamp);
    DBGU32_PRINT("total_days >> ",total_days);
    return timestamp;
}

typedef struct
{
    uint32_t timeout;
    char content[32];
}schedule_info;



uint32_t schedule_lasttime = 0;

schedule_info *schedule_list = NULL;
int schedule_sz = 0;
int schedule_use = 0;
void robot_main_schedule_init()
{
    schedule_sz = 10;
    schedule_list = (schedule_info*)calloc(sizeof(schedule_info),schedule_sz);
    schedule_use = 0;
}

void robot_main_schedule_add(char *datetime,char *content)
{
    if(schedule_use>=schedule_sz)
    {
        schedule_sz+=10;
        schedule_list = (schedule_info*)realloc(schedule_list,sizeof(schedule_info)*schedule_sz);
    }
    schedule_list[schedule_use].timeout = iso8601_to_timestamp(datetime);
    if(content==NULL)
        schedule_list[schedule_use].content[0] = '\0';
    else
        strcpy(schedule_list[schedule_use].content,content);
    schedule_use++;
    DBG_PRINT("schedule_add sz = ",schedule_use);
}

void robot_main_schedule_timer_clean()
{
    schedule_lasttime = 0;
}

void robot_main_schedule_loop()
{
    int i;

    uint32_t nowtime = time(NULL)+8*3600;
    uint32_t detime = nowtime - schedule_lasttime;
    //条件1：第一次检测
    //条件2：与上一次检测超过30秒
    //条件3：30秒的倍数 加上同秒排除
    if((schedule_lasttime==0)||(detime>=30)||((detime>0)&&(nowtime%30==0)))
    {
        for(i=0;i<schedule_use;i++)
        {
            DBG_PRINT("schedule idx = ",i);
            DBGU32_PRINT("nowtime = ",nowtime);
            DBGU32_PRINT("timeout = ",schedule_list[i].timeout);
            if(nowtime >= schedule_list[i].timeout)
            {
                DBG_PRINT("schedule timeout",0);
                ifly_aiui_schedule_send(schedule_list[i].content);
                schedule_use--;
                memcpy(&schedule_list[i],&schedule_list[schedule_use],sizeof(schedule_info));
                break;
            }
            else
            {
                DBGU32_PRINT("schedule wait >> ",schedule_list[i].timeout-nowtime);
            }
        }
        schedule_lasttime = nowtime;
    }
}


__attribute__((weak)) int aiui_get_vcn(void)
{
    DBG_PRINT("aiui_get_vcn >>",0);
    return 0;
}

__attribute__((weak)) void aiui_set_vcn(int idx)
{
    DBG_PRINT("aiui_set_vcn >>",idx);
}

__attribute__((weak)) int aiui_get_language(void)
{
    DBG_PRINT("aiui_get_language >>",0);
    return 0;
}

__attribute__((weak)) void aiui_set_language(int idx)
{
    DBG_PRINT("aiui_set_language >>",idx);
}

__attribute__((weak)) int aiui_get_persona(void)
{
    DBG_PRINT("aiui_get_persona >>",0);
    return 0;
}

__attribute__((weak)) void aiui_set_persona(int idx)
{
    DBG_PRINT("aiui_set_persona >>",idx);
}




static char robot_aiui_devid[128];
void aiui_set_devid(char *devid)
{
    if(devid)
        strcpy(robot_aiui_devid,devid);
}

char *aiui_get_devid()
{
    return robot_aiui_devid;
}


u32 robot_main_wait_target = 0;
void robot_main_wait_set(int waitms)
{
    robot_main_wait_target = ROBOT_GETTIME_MS() + waitms;
}

void robot_main_wait_do()
{
    //DBGU32_PRINT("target = ",robot_main_wait_target);
    while(1)
    {
        u32 nowtime = ROBOT_GETTIME_MS();
        //DBGU32_PRINT("now = ",ROBOT_GETTIME_MS());
        if(nowtime>=robot_main_wait_target)
            break;
    }
}
/*
uint32_t robot_main_ms = 0;
void robot_main_timer_ms(void *priv)
{
    robot_main_ms++;
}

uint32_t robot_main_get_ms()
{
    return robot_main_ms;
}
void robot_main_timer_init()
{
    sys_timeout_add(NULL,robot_main_timer_ms,1);
}
*/


