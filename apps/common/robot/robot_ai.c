//0其他，1AI，2聊天，3音乐，4动作
#include "asm/cpu.h"

#include "robot_main.h"
#include "robot_debug.h"
#include "robot_adapter.h"
#include "robot_bus.h"
#include "robot_ai.h"
#include "robot_control.h"



int robot_ai_flag_set = 0;
int robot_ai_flag_stop = 0;
int robot_ai_status = 0;
int ai_notips_flag = 0;

int turning_music_flag = 0;
int ai_music_ban = 0;

int ai_action_flag = 0;
int speak_end_flag = 0;
int robot_ai_mode = 0;
int ai_stop_tts_ban = 0;
int ai_switch_lock = 0;
int ai_music_flag = 0;
int ai_music_type = 0;
int TbzNetConfigFlag = 0;
int ai_again_record_flag = 0;
int puppy_wifi_playok = 0;
int ai_music_stop_dettime = 0;
int ota_update_flag = 0;
int tbz_net_err_flag = 0;
int action_music_flag = 0;
int net_connect_show = 0;
int net_disconnect_show = 0;
int asr_initok_flag = 0;
int ai_playpause_flag = 0;
//int asr_to_type = 0;
u32 ai2muisc_timer = 0;

extern void tbz_net_err_make();
extern void tbz_net_show_make();
extern void robot_ai_restart();


int robot_ai_switch = 0;
void robot_ai_init()
{
    turing_record_mode_set(0);
}

void robot_ai_switch_set(int sw)
{
    robot_ai_switch = sw;
}



int robot_ai_mode_onoff()
{
    //robot_main_debug_output("robot_ai_mode = ",robot_ai_mode);
    if(robot_ai_flag_stop)
        return 0;
    return ((robot_ai_mode==1)||(robot_ai_mode==2));
}

void robot_ai_mode_set(int mode)
{
    //robot_main_debug_output("robot_ai_mode_set ",mode);
    robot_ai_mode = mode;
    robot_asr_switch_set(3);
}


int robot_ai_errcnt = 0;

void robot_ai_err_clean()
{
    robot_ai_errcnt = 0;
}

int robot_ai_start()
{
    //robot_main_debug_output("robot_ai_start",1);
    robot_ai_flag_set = 1;
    wanson_asr_lock(0);
    return robot_ai_flag_set;
}



void robot_ai_start2()
{
    robot_ai_start();
}


void robot_ai_restart3()
{
    //robot_main_debug_output("robot_ai_restart",robot_ai_mode);
    if((robot_ai_mode==1)||(robot_ai_mode==2))
    {
        robot_ai_errcnt++;
        if(robot_ai_errcnt>=3)
        {
            ex_app_music_ai_listen_stop();
            robot_ai_stop(2);
        }
        else
        {
            robot_ai_flag_set = 1;
            ai_action_flag = 0;
            ai_music_ban = 5;
            ai_notips_flag = 1;
            ai_stop_tts_ban = 100;
            ex_app_music_ai_listen_stop();
            robot_ai_mode = 0;
            robot_main_debug_output("restart3 ok",0);
        }
    }
}

int robot_ai_stop(int tag)
{
    robot_main_debug_output("robot_ai_stop",tag);
    if((robot_ai_mode==1)||(robot_ai_mode==2))
    {
        robot_ai_flag_stop = 1;
        ai_stop_tts_ban = 100;
    }
    return robot_ai_flag_stop;
}


int puppy_aicode_make(int aicode)
{
    return 0;
}

int check_update_data(const char *url)
{
    #if 0
    int verb = 0;
    int verc = 0;
    int temp = 0;
    int have = 0;
    int ret = 0;
    int i;
    for(i=0;url[i];i++)
    {
        //robot_main_debug2_output("ih = ",i,have);
        if((url[i]>='0')&&(url[i]<='9'))
        {
            temp = temp*10 + (url[i]-'0');
            have = 1;
        }
        else
        {
            if(have)
            {
                verb = verc;
                verc = temp;
                temp = 0;
                have = 0;
            }
        }
    }
    if(have)
    {
        verb = verc;
        verc = temp;
    }
    DBG2_PRINT("ota_ver: ",verb,verc);
    DBG2_PRINT("cur_ver: ",TBZ_PUPPY_VERSION_B,TBZ_PUPPY_VERSION_C);
    if(verb>TBZ_PUPPY_VERSION_B)
        ret = 1;
    if((verb==TBZ_PUPPY_VERSION_B)&&(verc>TBZ_PUPPY_VERSION_C))
        ret = 1;
    //DBG2_PRINT("ret = ",ret);
    if(ret==0)
        puppy_set_update_clr();
    return ret;
    #endif
    return 0;
}

int puppy_wifi_ok_judge()
{
    //DBG_PRINT("puppy_wifi_playok = ",puppy_wifi_playok);
    return puppy_wifi_playok;
}


int ai_record_mode = 1;

static int ai_listen_start(int priv)
{
    clr_ai_code();
    ai_again_record_flag = 0;
    if (ex_app_music_ai_listen_start()!=0)
    {
        robot_ai_flag_stop = 1;
    }
    return 0;
}

extern void turning_app_set_record_enable(int frame_giveup);

static int asr_listen_enable(int priv)
{
    //robot_main_debug_output("record_enable",priv);
    turning_app_set_record_enable(0);
    robot_emoji_mic_start();
}

static int asr_listen_start2(int priv)
{
    //robot_emoji_emoji_set(EMOJI_NORMAL,0);
    turing_record_mode_set(ai_record_mode);
    if(ai_record_mode==0)
    {
        //robot_emoji_emoji_set(EMOJI_NORMAL,0);
        mdelay(80);
        os_set_play_audio("VRKey.mp3", ai_listen_start);

    }
    else
    {
        ai_listen_start(0);
        puppy_delay(20);
        os_set_play_audio("VRKey.mp3", asr_listen_enable);
    }

    return 0;
}

extern void asr_speak_end_set();

void robot_wifi_status_make()
{
    if(TbzNetConfigFlag)
    {
        if(TbzNetConfigFlag==1)
        {
            if(!is_in_config_network_state())
            {
                //robot_emoji_emoji_set(EMOJI_WIFI,0);
                robot_asr_switch_set(2);
                os_set_net_mode();
            }
        }
        else if(TbzNetConfigFlag==2)
        {
            if(is_in_config_network_state())
            {
                os_set_net_mode();
                robot_asr_switch_set(3);
                //robot_emoji_emoji_set(EMOJI_NORMAL,0);
            }
        }
        TbzNetConfigFlag = 0;
    }
}

void robot_ai_loop()
{
    static int playidx = 0;

    if(robot_ai_flag_set)
    {
        ai_stop_tts_ban = 0;

        ai_again_record_flag = 0;
        //if (!__this->ai_connected || !__this->net_connected)
        if(!app_music_check_wifi())
        {
            //robot_emoji_emoji_set(EMOJI_WIFING,6);
            os_set_play_audio("035ai.mp3", NULL);
            return ;
        }
        //robot_emoji_emoji_set(EMOJI_AI,0);
        //Action_set(Sit_down2);
        robot_asr_switch_set(2);
        robot_audio_play_break();
        robot_ai_mode_set(1);
        //robot_emoji_emoji_set(EMOJI_NORMAL,0);
        robot_main_debug_output("robot_ai_status = ",robot_ai_status);
#if 0
        switch(robot_ai_status)
        {
        case 0:
            ex_app_music_ai_listen_stop();
            app_music_play_voice_prompt("AIStart.mp3", asr_listen_start2);
            playidx = 0;
            robot_ai_status = 1;
            break;
        case 1:
            ex_app_music_ai_listen_stop();
            if(playidx==0)
                app_music_play_voice_prompt("AIStart.mp3", asr_listen_start2);
            else
                app_music_play_voice_prompt("AIStart.mp3", asr_listen_start2);
            playidx = 1 - playidx;
            break;
        }
#else
        ex_app_music_ai_listen_stop();
        if(ai_notips_flag)
        {
            ai_notips_flag = 0;
            asr_listen_start2(0);
        }
        else
        {
            os_set_play_audio("AIStart.mp3", asr_listen_start2);
        }

        robot_ai_status = 1;
        robot_ai_flag_set = 0;
#endif
    }

    if(robot_ai_flag_stop)
    {
        robot_ai_status = 0;
        ex_app_music_ai_listen_stop();
        ex_dac_play_break();
        robot_ai_mode_set(0);
        mdelay(20);
        robot_asr_switch_set(3);
        robot_ai_flag_stop = 0;
        extern int fast_voice_ban_flag;
        DBG_PRINT("ai_stop fast_voice_ban_flag = ",fast_voice_ban_flag);
        if(!fast_voice_ban_flag)
        {
            os_set_play_audio("AIStop.mp3", NULL);
            //robot_emoji_emoji_set(EMOJI_HAPPY,2);
        }
        fast_voice_ban_flag = 0;
        ai_action_flag = 0;
    }
    if(ai_stop_tts_ban>0)
    {
        ai_stop_tts_ban--;
        mdelay(10);
    }
    extern int aiasr_switch_lock;
    if(aiasr_switch_lock)
    {
        aiasr_switch_lock--;
        if(aiasr_switch_lock<0)
            aiasr_switch_lock = 0;
        mdelay(20);
    }
    tbz_net_err_make();
    tbz_net_show_make();
    if((ai_action_flag==2)||(speak_end_flag))
    {
        ai_action_flag = 0;
        speak_end_flag = 0;
        extern int last_action_show_flag;
        if(last_action_show_flag)
            robot_ai_restart();
        else
            asr_speak_end_set();
    }
}


void tbz_net_err_send(int flag)
{
    tbz_net_err_flag = flag;
    DBG_PRINT("tbz_net_err_send : ",flag);
}

void tbz_net_err_make()
{
    if(tbz_net_err_flag)
    {
        DBG_PRINT("tbz_net_err_flag = ",tbz_net_err_flag);
        switch(tbz_net_err_flag)
        {
        case 1://音乐控制
            break;
        case 2://图片识别
            break;
        case 3://AI语音识别
            robot_ai_restart3();//重启试试
            break;
        }
        tbz_net_err_flag = 0;
    }
}



void tbz_net_show_make()
{

    //extern int net_connect_show;
    //extern int net_disconnect_show;
    get_device_id(deviceid);

    //DBG_PRINT("net_disconnect_show",net_disconnect_show);
    if(net_connect_show)
    {
            STR_PRINT("WIFI_CFGOK\r\n");
            DBG_PRINT("deviceid ",deviceid);
            net_connect_show = 0;
            os_set_play_audio("NetCfgSucc.mp3", NULL);
            app_wifistatus_send(1);
    }
    if(net_disconnect_show)
    {
            net_disconnect_show = 0;
            app_wifistatus_send(0);
    }
}

void asr_speak_end_set()
{
    extern int ai_stop_tts_ban;
    if(ai_stop_tts_ban)
    {
        return;
    }
    if(ai_again_record_flag==1)
    {
        //ai_listen_start(0);
        //action_delay(20);
        //app_music_play_voice_prompt("VRKey.mp3", asr_listen_enable);

        turing_record_mode_set(ai_record_mode);

        if(ai_record_mode==0)
        {
            os_set_play_audio("VRKey.mp3", ai_listen_start);
        }
        else
        {
            ai_listen_start(0);
            puppy_delay(20);
            os_set_play_audio("VRKey.mp3", asr_listen_enable);
        }
    }
    ai_again_record_flag = 0;
}

void asr_speak_end_set2()
{
    extern int ai_stop_tts_ban;

    if(robot_ai_flag_stop)
    {
        robot_ai_flag_stop = 0;
    }
    if((robot_ai_mode==0)||(robot_ai_mode==3))
    {
        robot_ai_start();
    }
    else
    {
        //asr_listen_start(0);
        //action_delay(20);
        //app_music_play_voice_prompt("VRKey.mp3", asr_listen_enable);

        turing_record_mode_set(ai_record_mode);

        if(ai_record_mode==0)
        {
            app_music_play_voice_prompt("VRKey.mp3", ai_listen_start);
        }
        else
        {
            ai_listen_start(0);
            action_delay(20);
            app_music_play_voice_prompt("VRKey.mp3", asr_listen_enable);
        }
    }
}

int robot_ai_mode_get()
{
    return ((robot_ai_mode==1)||(robot_ai_mode==2)||(robot_ai_mode==5));
}

void speak_end_flag_set()
{
    speak_end_flag = 1;
}

void robot_ai_restart()
{
    if((robot_ai_mode==1)||(robot_ai_mode==2))
    {
        robot_ai_flag_set = 1;
        ai_action_flag = 0;
        ai_music_ban = 5;
        ai_stop_tts_ban = 100;
    }
}

void robot_ai_pause()
{
    //robot_main_debug_output("robot_ai_pause",robot_ai_mode);
    if((robot_ai_mode==1)||(robot_ai_mode==2))
    {
        ai_stop_tts_ban = 100;
    }
}

int robot_ai_stop2(int tag)
{
    int last_code;
    last_code = get_ai_last_code();
    //robot_main_debug_output("last_code",last_code);
    if((robot_ai_mode==1)||(robot_ai_mode==2))
    {
        if(last_code==20012)//百科
        {
            //robot_main_debug_output("ai_restart??",20012);
            robot_ai_pause();
            robot_ai_restart();
        }
        else
        {
            robot_ai_stop(5);
        }
        return 1;
    }
    return 0;
}

void robot_ai_mode_switch()
{
    //robot_main_debug_output("robot_ai_mode = ",robot_ai_mode);
    ai_switch_lock = 10;
    if((robot_ai_mode==0)||(robot_ai_mode==3))
    {
        robot_ai_start();
    }
    else if((robot_ai_mode==1)||(robot_ai_mode==2))
    {
        robot_ai_stop2(1);
    }
    else if(robot_ai_mode==5)
    {
       robot_ai_stop2(2);
    }
}

//ai asr 的restart下周要全部过一次。。8547

void robot_ai_restart2()
{
    //robot_main_debug_output("robot_ai_restart",robot_ai_mode);
    if((robot_ai_mode==1)||(robot_ai_mode==2))
    {
        robot_ai_flag_set = 1;
        ai_action_flag = 0;
        ai_music_ban = 5;
        ai_stop_tts_ban = 100;
        ex_app_music_ai_listen_stop();
        robot_ai_mode = 0;
        robot_main_debug_output("app_music_ai_listen_stop ok",0);
    }
}

#include "wifi/wifi_connect.h"

void wifi_scan_show()
{
    int i;
    unsigned int sta_ssid_num=24;
    char strinfo[128];
    struct wifi_scan_ssid_info *sta_ssid_info;
    sta_ssid_info = wifi_get_scan_result(&sta_ssid_num);
    DBG_PRINT("wifi cnt = ",sta_ssid_num);
    for (i = 0; i < sta_ssid_num; i++)
    {
        sprintf(strinfo,"%d\t%s\t%d\t%d\t%d\t",i,sta_ssid_info[i].ssid,
                sta_ssid_info[i].rssi,sta_ssid_info[i].SignalQuality,sta_ssid_info[i].SignalStrength);
        //DBGSTR_PRINT("ssid >>",sta_ssid_info[i].ssid);
        DBGSTR_PRINT(">>\t",strinfo);
        //DBG_PRINT("SignalStrength >>",sta_ssid_info[i].SignalStrength);
        //DBG_PRINT("SignalQuality >>",sta_ssid_info[i].SignalQuality);
    }
    if(sta_ssid_info)
        free(sta_ssid_info);
}

void wifi_scan_set()
{
    int scan_req = wifi_scan_req();
    DBG_PRINT("wifi scan_req = ",scan_req);
}

void wifi_rssi_get()
{
    u32 rate;
    char rssi = wifi_get_rssi();
    char cqi = wifi_get_cqi();
    DBG_PRINT("wifi rssi = ",rssi);
    DBG_PRINT("wifi cqi = ",cqi);
    rate = wifi_get_upload_rate();
    DBGU32_PRINT("wifi upload_rate = ",rate);
    rate = wifi_get_download_rate();
    DBGU32_PRINT("wifi upload_rate = ",rate);
}

void wifi_mode_info_get()
{
    struct wifi_mode_info info;
    info.mode = NONE_MODE;
    wifi_get_mode_cur_info(&info);
    DBG_PRINT("WiFi Mode = ",info.mode);
    DBGSTR_PRINT("SSID = ",info.ssid);
    DBGSTR_PRINT("PWD = ",info.pwd);
}

//wifi test

bool exit_config = false;
bool wife_ssid_pwd_input(u8* data,int sz)
{
    if(sz <= 0) {
        STR_PRINT("No data received\n");
        return false;
    }

    if(sz == 1 && data[0] == '3') {
        STR_PRINT("Exiting configuration mode...\n");
        exit_config = true;
        return false;
    }else if(sz == 1 && data[0] == '2'){
        STR_PRINT("SUCCES\n");
        exit_config = false;
        return true;
    }

    char ssid[128], pwd[128];
    int ssid_index = 0, pwd_index = 0;
    bool is_ssid = true;
    bool is_format_correct = false;

    for(int i = 0; i < sz; i++) {

        if(data[i] == '#') {
            if(i != 0 && i != sz - 1) { // Check the format: '#' should not be the first or the last character.
                is_ssid = false;
                ssid[ssid_index] = '\0';
                is_format_correct = true;
            }
            continue;
        }

        if(is_ssid) {
            ssid[ssid_index++] = data[i];
        } else {
            pwd[pwd_index++] = data[i];
        }
    }
    pwd[pwd_index] = '\0';

    if(!is_format_correct) {
        STR_PRINT("Please enter your SSID and password separated by '#'\r\n");
    } else {
        // Pass the ssid and pwd to the wifi_cfg_test function.
        STRDBG_PRINT("SSID: ", ssid);
        STRDBG_PRINT("Password: ", pwd);
        //wifi_cfginfo_set(ssid, pwd);

        // Check network state after attempting configuration
        if(!os_get_net_config_mode()) {
            os_set_net_config_mode(-1);

            // Optionally, check the network state again and output debug message
            if(!os_get_net_config_mode()) {
                STR_PRINT("Network configuration failed. Retrying...\r\n");
            } else {
                STR_PRINT("Network configuration successful.\r\n");
            }
        }
    }
    return is_format_correct;
}

void robot_ai_wificfg_setmode()
{
    DBG_PRINT("robot_ai_wificfg_setmode",0);
    os_set_net_config_mode(-1);
}

void robot_ai_wificfg_setinfo(char* ssid , char* pwd)
{
    char cfg_buf[256];
    snprintf(cfg_buf, sizeof(cfg_buf), "{\"ssid\":\"%s\", \"pass\":\"%s\"}", ssid, pwd);
    //DBG_PRINT("wifi_info = ",cfg_buf);
    extern int bt_net_config_set_ssid_pwd(const char *data);
    int res = bt_net_config_set_ssid_pwd(cfg_buf);
    DBG_PRINT("net_config = ",res);
    if(res == -1)
        DBG_PRINT("net_config not ready",-2);
}

#define TBZ_WIFI_SSID "TBZ_Debug"
#define TBZ_WIFI_PWD  "Pwd1314520"

void robot_ai_wificfg_tbz()
{
    robot_ai_wificfg_setinfo(TBZ_WIFI_SSID,TBZ_WIFI_PWD);
}

void robot_ai_listen()
{
    extern int ex_app_music_ai_listen_start();
    int ret = ex_app_music_ai_listen_start();
    DBG_PRINT("robot_ai_listen >>",ret);
}
