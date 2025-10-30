#include "robot_main.h"
#include "robot_debug.h"
#include "robot_adapter.h"
#include "robot_util.h"
#include "robot_cmd.h"
#include "robot_head.h"
#include "robot_uart.h"
#include "event/key_event.h"
#include "json_tools.h"  // 添加JSON工具头文件
#include "xz_main.h"


// 指令信息结构体
typedef struct {
    const char *command;      // 指令字符串
    int param_count;          // 参数个数
    void (*func)(char **);    // 对应的函数指针
} CommandInfo;


//*** 指令加在这里 ***


void robot_blink_send(int blink_type,int blink_count)
{
    uint16_t data[2];
    data[0] = blink_type;
    data[1] = blink_count;
    head_msg_send(MSG_BLINK,2,data);
}


void robot_smile_send(int smile_type)
{
    uint16_t data[1];
    data[0] = smile_type;
    head_msg_send(MSG_SMILE,1,data);
}

void robot_speak_send(int mode,int speed,int amplitude)
{
    uint16_t data[3];
    data[0] = mode;
    data[1] = speed;
    data[2] = amplitude;
    head_msg_send(MSG_SPEAK,3,data);
}

void robot_lick_send(int mode,int neck)
{
    uint16_t data[2];
    data[0] = mode;
    data[1] = neck;
    head_msg_send(MSG_LICK,2,data);
}



void robot_rub_send(int dir,int count,int smile)
{
    uint16_t data[3];
    data[0] = dir;
    data[1] = count;
    data[2] = smile;
    head_msg_send(MSG_RUB,3,data);
}

void robot_flagset_send(int idx,int flag)
{
    uint16_t data[2];
    data[0] = idx;
    data[1] = flag;
    head_msg_send(MSG_SET,2,data);
}

void robot_debug_send(int data1,int data2)
{
    uint16_t data[2];
    data[0] = data1;
    data[1] = data2;
    head_msg_send(MSG_DEBUG,2,data);
}

void robot_sleep_send(int ms)
{
    ROBOT_DELAY_MS(ms);
}

void robot_speak_status_wait(int status,int once_ms)
{
    while(1)
    {
        if(get_head_status()!=1)
            break;
        if(get_speak_status()==status)
            break;
        robot_sleep_send(once_ms);
    }
    //robot_sleep_send(once_ms);
}

void robot_speakn_send(int count)
{
    int i;
    for(i=0;i<count;i++)
    {
        robot_speak_send(1,14,80);
        robot_speak_status_wait(4,25);
        robot_speak_send(2,16,-10);
        robot_speak_status_wait(6,25);
    }
    robot_speak_send(0,0,0);
    robot_sleep_send(50);
}


void robot_command_hello(char **tokens) {
    DBG_PRINT("Hello World TH01_V0.4 20250906",0);
}

void robot_command_tag(char **tokens) {
    char * tag_text = tokens[1];
    //printf("执行 smile: 参数=%d\n", para0);
    DBG_PRINTF("========== %s ==========",tag_text);
    // 发送标签消息到WebSocket服务器（使用listen类型）
    send_tag_to_websocket(tag_text);

}

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



void robot_command_flagset(char **tokens) {
    int para0 = atoi(tokens[1]);
    int para1 = atoi(tokens[2]);
    //printf("执行 speak: 参数1=%d, 参数2=%d, 参数3=%d\n", para0, para1, para2);
    robot_flagset_send(para0, para1);
}

void robot_command_headdebug(char **tokens)
{
    int para0 = atoi(tokens[1]);
    int para1 = atoi(tokens[2]);
    robot_debug_send(para0,para1);
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

void robot_command_speakd(char **tokens) {
    int para0 = atoi(tokens[1]);
    //printf("执行 sleep: 参数=%d\n", para0);
    robot_main_speak_once(para0);
    //robot_main_speak_once(para0);
}

void robot_command_speakwait(char **tokens) {
    int para0 = atoi(tokens[1]);
    int para1 = atoi(tokens[2]);
    //printf("执行 sleep: 参数=%d\n", para0);
    robot_speak_status_wait(para0,para1);
    //robot_main_speak_once(para0);
}

void robot_command_readservo(char **tokens) {
    robot_action_ts15_posi_read();
}



// 映射表数组
static const StringValueMap mode_mappings[] = {
    {"cmd", MAININPUT_MODE_CMD},
    {"data", MAININPUT_MODE_DATA},
    {"config", MAININPUT_MODE_CONFIG},
    {"cfg", MAININPUT_MODE_CONFIG},
    {"app", MAININPUT_MODE_APP},
    {"2p4g", MAININPUT_MODE_RF2P4G},
    {"rf2p4g", MAININPUT_MODE_RF2P4G},
    {"bus", MAININPUT_MODE_UARTBUS}
};


void robot_command_modeset(char **tokens) {
    char *input = tokens[1];
    int para0;

    // 检查输入是否为数字
    int is_number = 1;
    for (int i = 0; input[i] != '\0'; i++) {
        if (!isdigit((unsigned char)input[i])) {
            is_number = 0;
            break;
        }
    }

    if (is_number) {
        para0 = atoi(input);
    } else {
        para0 = MAP_STRING2VALUE(input,mode_mappings);
    }
    if(para0!=-1)
        robot_main_input_mode_set(para0);
}

void robot_command_modeDATA(char **tokens) {
    robot_main_input_mode_set(MAININPUT_MODE_DATA);
}

void robot_command_headuse(char **tokens) {
    int para0 = atoi(tokens[1]);
    robot_head_use_set(para0);
}

void robot_command_michiko_setup(char **tokens)
{
    os_set_volume(10,0,0);
    robot_key_use_set(2);
    robot_head_init();
    head_flag_set(FLAG_BLINK,1);
    head_flag_set(FLAG_SMILE,1);
    robot_head_recover();
    ROBOT_DELAY_MS(100);
    robot_head_timeset();
    robot_head_use_set(1);
    os_set_play_audio("michiko2.mp3",NULL);

}

void robot_command_develop_setup(char **tokens)
{
    os_set_volume(5,0,0);
    //robot_key_use_set(2);
    robot_head_init();
    robot_head_recover();
    ROBOT_DELAY_MS(100);
    robot_head_timeset();
    robot_head_use_set(1);
    //os_set_play_audio("develop.mp3",NULL);
}



void robot_command_vadmode_setup(char **tokens)
{
    //DBG_PRINT("toggle_duplex_mode",0); // 打印日志
    //toggle_duplex_mode();

    //模拟按键的方式
//    struct key_event key = {0};
//    key.action = KEY_EVENT_LONG;
//    key.value = KEY_MODE;
//    key.type = SYS_KEY_EVENT;
//    key_event_notify(SYS_KEY_EVENT, &key);

}


void robot_comand_headcmd(char **tokens) {
    char *headcmd = tokens[1];
    DBG_PRINTF("headcmd : %s",headcmd);
    if(strcmp(headcmd,"init")==0)
    {
        robot_head_init();
    }
    else if((strcmp(headcmd,"timeset")==0)||(strcmp(headcmd,"settime")==0))
    {
        robot_head_timeset();
    }
    else if(strcmp(headcmd,"recover")==0)
    {
        robot_head_recover();
    }
    else if(strcmp(headcmd,"read")==0)
    {
        robot_head_read();

    }
    else if(strcmp(headcmd,"start")==0)
    {
        robot_head_use_set(1);
    }
    else if(strcmp(headcmd,"stop")==0)
    {
        robot_head_use_set(0);
    }
    else
    {
        DBG_PRINTF("Unknow Head Command");
    }
}

void robot_command_volume(char **tokens) {
    int para0 = atoi(tokens[1]);
    DBG_PRINTF("volume set : %d",para0);
    os_set_volume(para0,0,0);
    os_set_tone_ban(1);
    os_set_play_audio("PowerOn.mp3",NULL);
}


// 映射表数组
static const StringValueMap llm_command_mappings[] = {
    {"mode1", LLM_MODE_SINGLE},
    {"single", LLM_MODE_SINGLE},
    {"one", LLM_MODE_SINGLE},
    {"mode2", LLM_MODE_MULTI},
    {"multi", LLM_MODE_MULTI},
    {"mode3", LLM_MODE_DUPLEX},
    {"duplex", LLM_MODE_DUPLEX},
    {"continue", LLM_MODE_DUPLEX}
};

// 计算映射表中的条目数
#define LLM_COMMAND_COUNT (sizeof(llm_command_mappings) / sizeof(llm_command_mappings[0]))

void robot_command_llm(char **tokens)
{
    int cmd = MAP_STRING2VALUE(tokens[1],llm_command_mappings);
    switch(cmd)
    {
    case LLM_MODE_SINGLE:
        llm_set_interactive_mode(1);
        break;
    case LLM_MODE_MULTI:
        llm_set_interactive_mode(2);
        break;
    case LLM_MODE_DUPLEX:
        llm_set_interactive_mode(3);
        break;
    }
}

// 映射表数组
static const StringValueMap breathe_command_mappings[] = {
    {"off", 0},
    {"0", 0},
    {"close", 0},
    {"on", 1},
    {"1", 1},
    {"open", 1},
};



void robot_command_breathe(char **tokens)
{
    int cmd = MAP_STRING2VALUE(tokens[1],breathe_command_mappings);
    if(cmd!=-1)
        robot_main_breathe_set(cmd);
}


// 播放状态管理结构体
typedef struct {
    int type;
    int idx;
    int current_play_index;
} PlayState;

// 全局播放状态
static PlayState g_play_state = {0, 0, 0};

// 音频播放配置结构体
typedef struct {
    const char *name;
    const char *file_template;
    int file_count;
} AudioConfig;

// 类型映射表
static StringValueMap type_mappings[] = {
    {"1", 1},
    {"system", 1},
    {"2", 2},
    {"touch", 2}
};

// 索引映射表
static StringValueMap idx_mappings[] = {
    // system类型
    {"1", 1},
    {"open", 1},
    {"poweron", 1},
    {"kaiji", 1},
    {"2", 2},
    {"develop", 2},
    {"kaifa", 2},
    {"3", 3},
    {"michiko", 3},
    {"meizhizi", 3},
    // touch类型
    {"2", 2},
    {"lf", 2},
    {"left", 2},
    {"leftface", 2},
    {"zuolian", 2},
    {"3", 3},
    {"rf", 3},
    {"right", 3},
    {"rightface", 3},
    {"youlian", 3},
    {"4", 4},
    {"chin", 4},
    {"xiaba", 4},
    {"6", 6},
    {"fore", 6},
    {"forehead", 6},
    {"etou", 6},
    {"8", 8},
    {"universal", 8},
    {"tongyong", 8}
};

// 音频文件配置
static AudioConfig audio_configs[] = {
    // system类型
    {"system_poweron", "poweron.mp3", 1},
    {"system_develop", "develop.mp3", 1},
    {"system_michiko", "michiko%d.mp3", 4},

    // touch类型
    {"touch_leftface", "touch2_%d.mp3", 5},
    {"touch_rightface", "touch3_%d.mp3", 5},
    {"touch_chin", "touch4_%d.mp3", 5},
    {"touch_forehead", "touch6_%d.mp3", 5},
    {"touch_universal", "touch8_%d.mp3", 5}
};

// 根据类型字符串获取类型值
int get_play_type(const char *type_str) {
    return MAP_STRING2VALUE(type_str, type_mappings);
}

// 根据索引字符串获取索引值
int get_play_idx(const char *idx_str) {
    return MAP_STRING2VALUE(idx_str, idx_mappings);
}

// 生成音频文件名
char* generate_audio_filename(const char *template, int index, char *buffer, int buffer_size) {
    if (template == NULL || buffer == NULL) return NULL;

    // 检查模板中是否包含 %d
    if (strstr(template, "%d") != NULL) {
        snprintf(buffer, buffer_size, template, index);
    } else {
        snprintf(buffer, buffer_size, "%s", template);
    }

    return buffer;
}

// 获取音频配置
AudioConfig* get_audio_config(int type, int idx) {
    const char *config_name = NULL;

    // 根据类型和索引确定配置名称
    switch (type) {
        case 1:  // system
            switch (idx) {
                case 1: config_name = "system_poweron"; break;
                case 2: config_name = "system_develop"; break;
                case 3: config_name = "system_michiko"; break;
                default: return NULL;
            }
            break;

        case 2:  // touch
            switch (idx) {
                case 2: config_name = "touch_leftface"; break;
                case 3: config_name = "touch_rightface"; break;
                case 4: config_name = "touch_chin"; break;
                case 6: config_name = "touch_forehead"; break;
                case 8: config_name = "touch_universal"; break;
                default: return NULL;
            }
            break;

        default:
            return NULL;
    }

    // 查找对应的音频配置
    for (int i = 0; i < sizeof(audio_configs) / sizeof(audio_configs[0]); i++) {
        if (strcmp(audio_configs[i].name, config_name) == 0) {
            return &audio_configs[i];
        }
    }

    return NULL;
}

// 主播放函数
void robot_command_play(char **tokens) {
    if (tokens == NULL || tokens[1] == NULL || tokens[2] == NULL) {
        DBG_PRINTF("Error: Invalid parameters");
        return;
    }

    int type = get_play_type(tokens[1]);
    int idx = get_play_idx(tokens[2]);

    if (type == -1 || idx == -1) {
        DBG_PRINTF("Error: Invalid type or index");
        return;
    }

    // 检查是否需要重置播放状态
    if (g_play_state.type != type || g_play_state.idx != idx) {
        g_play_state.type = type;
        g_play_state.idx = idx;
        g_play_state.current_play_index = 1;
    }

    // 获取音频配置
    AudioConfig *config = get_audio_config(type, idx);
    if (config == NULL) {
        DBG_PRINTF("Error: No audio config found for type=%d, idx=%d", type, idx);
        return;
    }

    // 生成文件名并播放
    char filename[32];
    int play_index = g_play_state.current_play_index;

    generate_audio_filename(config->file_template, play_index, filename, sizeof(filename));

    DBG_PRINTF("Playing: %s", filename);
    os_set_play_audio(filename, NULL);

    // 更新播放索引
    g_play_state.current_play_index++;
    if (g_play_state.current_play_index > config->file_count) {
        g_play_state.current_play_index = 1;
    }
}

void robot_command_asrmode(char **tokens)
{
    char *mode = tokens[1];
    if(strcmp(mode,"..")==0)
    {

    }
}

StringValueMap busCmdMap[] = {
        {"write", 100},
        {"debug", 200},
        {"dbg", 200},
        {"mode", 300},
        {"off", UART_DBG_OFF},
        {"on", UART_DBG_ON},
        {"onlock", UART_DBG_OFF},
        {"auto", BUS_DIRMODE_AUTO},
        {"manual", BUS_DIRMODE_MANUAL},
        {"output", BUS_DIRMODE_OUTPUT},
        {"out", BUS_DIRMODE_OUTPUT},
        {"close", BUS_DIRMODE_CLOSE},
        {"in", BUS_DIRMODE_CLOSE},
        {"0", 0},
        {"1", 1},
        {"2", 2},
        {"3", 3}
};



void robot_command_buswrite(char input[])
{
    //to do
    uint8_t data[40];
    int datalen = hex_string_to_uint8_array(input,data);
    if(datalen)
        BUS_WRITE(data,datalen);

}
void robot_command_bus(char **tokens)
{
    char *typestr = tokens[1];
    char *datastr = tokens[2];
    int typeint = MAP_STRING2VALUE(typestr,busCmdMap);
    int dataint = MAP_STRING2VALUE(datastr,busCmdMap);
    switch(typeint)
    {
    case 100:
        robot_command_buswrite(datastr);
        break;
    case 200:
        robot_uart_bus_debug_set(dataint);
        break;
    case 300:
        robot_uart_bus_dirmode_set(dataint);
        break;
    default:
        INFO_PRINTF("unknow bus command.");
    }
    #if 0

    if(strcmp(tokens,"write")==0)
    {
        robot_command_buswrite(data);
    }
    else if((strcmp(tokens,"debug")==0)||(strcmp(tokens,"dbg")==0))
    {
        if((strcmp(data,"0")==0)||(strcmp(data,"off")==0))
            robot_uart_bus_debug_set(UART_DBG_OFF);
        else if((strcmp(data,"1")==0)||(strcmp(data,"on")==0))
            robot_uart_bus_debug_set(UART_DBG_ON);
        else
            DBG_PRINTF("Unknow data");
    }
    else if(strcmp(tokens,"mode")==0)
    {
        if((strcmp(data,"0")==0)||(strcmp(data,"off")==0))
            robot_uart_bus_debug_set(UART_DBG_OFF);
        else if((strcmp(data,"1")==0)||(strcmp(data,"on")==0))
            robot_uart_bus_debug_set(UART_DBG_ON);
        else
            DBG_PRINTF("Unknow data");
    }
    #endif
}

void robot_command_keyuse(char **tokens) {
    int para0 = atoi(tokens[1]);
    //robot_head_use_set(para0);
    robot_key_use_set(para0);
}

void robot_command_key(char **tokens) {
    char *cmd = tokens[1];
    //robot_head_use_set(para0);
    if(strcmp(cmd,"debugonce")==0)
        robot_ttp_debug_set(1);
    else if(strcmp(cmd,"debugstart")==0)
        robot_ttp_debug_set(3);
    else if(strcmp(cmd,"debugstop")==0)
        robot_ttp_debug_set(0);
    else if(strcmp(cmd,"off")==0)
        robot_key_use_set(0);
    else if(strcmp(cmd,"debug")==0)
        robot_key_use_set(1);
    else if(strcmp(cmd,"use")==0)
        robot_key_use_set(2);
}



void robot_command_ts15read(char **tokens)
{
    char *endptr;
    int id, add, length;

    int errno = 0;
    id = (int)strtol(tokens[1], &endptr, 10);
    if (errno != 0 || *endptr != '\0') {
        // 处理转换错误
        return;
    }

    errno = 0;
    add = (int)strtol(tokens[2], &endptr, 16);
    if (errno != 0 || *endptr != '\0') {
        // 处理转换错误
        return;
    }

    errno = 0;
    length = (int)strtol(tokens[3], &endptr, 10);
    if (errno != 0 || *endptr != '\0') {
        // 处理转换错误
        return;
    }
    TS15_IDUserRead((int)id, (int)add, (int)length);
}

void robot_command_ts15write(char **tokens)
{
    char *endptr;
    long id, add, length;
    uint8_t data[40];
    int errno = 0;
    id = (int)strtol(tokens[1], &endptr, 10);
    if (errno != 0 || *endptr != '\0') {
        // 处理转换错误
        return;
    }

    errno = 0;
    add = (int)strtol(tokens[2], &endptr, 16);
    if (errno != 0 || *endptr != '\0') {
        // 处理转换错误
        return;
    }

    int datalen = hex_string_to_uint8_array(tokens[3],data);
    if(datalen)
        TS15_IDUserWrite(id, add, datalen, data);
}


//***********************

// 指令表
CommandInfo command_table[] = {
    {"hello",0, robot_command_hello},
    {"tag",1,robot_command_tag},
    {"smile", 1, robot_command_smile},//type
    {"blink", 2, robot_command_blink},//type times
    {"rub", 3, robot_command_rub},
    {"lick", 2, robot_command_lick},//
    {"speak", 3, robot_command_speak},//type: 1 open 2 close speed:8-16:amplitude:60-120
    {"auto", 2, robot_command_flagset},
    {"headdebug", 2, robot_command_headdebug},
    {"sleep", 1, robot_command_sleep}, // ms
    {"speakn", 1, robot_command_speakn}, // times
    {"speakd", 1, robot_command_speakd}, // times
    {"speakw", 2, robot_command_speakwait},// status ms
    {"speakwait", 2, robot_command_speakwait},// status ms
    {"head",1,robot_comand_headcmd}, // command
    {"headuse", 1, robot_command_headuse}, // switch
    {"touchuse", 1, robot_command_keyuse}, // switch
    {"touch", 1, robot_command_key}, //command
    {"key", 1, robot_command_key},
    {"readservo", 0, robot_command_readservo},
    {"modeset", 1, robot_command_modeset},
    {"bus", 2, robot_command_bus},
    {"ts15read", 3, robot_command_ts15read},
    {"ts15write", 3, robot_command_ts15write},
    {"michiko",0,robot_command_michiko_setup},
    {"develop",0,robot_command_develop_setup},
    {"volume",1,robot_command_volume},
    {"llm",1,robot_command_llm},
    {"ai",1,robot_command_llm},
    {"play",2,robot_command_play},
    {"9",0,robot_command_modeDATA},//兼容原来的CMD模式，直接输入'9',就能进入DATA模式,兼容原有上位机工具
    {"breathe",1,robot_command_breathe},
    //{"vadmode",0,robot_command_vadmode_setup},
    // 可以轻松添加新指令
    // {"new_command", 2, robot_command_new},
};
// 获取指令表中的指令数量
#define COMMAND_COUNT (sizeof(command_table) / sizeof(CommandInfo))


void robot_command_make(char *command) {
    char *token = strtok(command, " ");
    static char *tokens[100];  // 存储解析后的令牌  用堆空间，避免吧栈压爆了。
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

/*
讲话
tag speak1 speak 1 16 95 speakw 4 25 speak 2 18 -48 speakw 6 25 tag speak2 speak 1 16 95 speakw 4 25 speak 2 18 -48 speakw 6 25 speak 0 0 0
*/
