#include "robot_main.h"
#include "robot_control.h"
#include "robot_debug.h"
#include "robot_adapter.h"


//播放音频

__attribute__((weak)) void os_set_unmute_pin(int unmute)
{

}
__attribute__((weak)) void os_set_play_audio(const char *name,void *play_end)
{
    STRDBG_PRINT("os_set_play_audio >> ",name);
}


__attribute__((weak)) void os_set_play_audio_resume(const char *name,int resume)
{
    STRDBG_PRINT("os_set_play_audio_resume >> ",name);
}


__attribute__((weak)) int os_get_play_status(void)
{
    STRDBG_PRINT("os_get_play_status >>",0);
    return 0;
}

__attribute__((weak)) int os_set_play_break(void)
{
    STRDBG_PRINT("os_set_play_break >>",0);
    return 0;
}


__attribute__((weak)) int os_get_tone_status()
{
    return 0;
}

__attribute__((weak)) void os_set_tone_status(int tone)
{

}

__attribute__((weak)) void os_set_tone_ban(int banflag)
{

}


__attribute__((weak)) void os_set_volume(int volume,int play,int save)
{
    DBG_PRINT("set volume >>",volume);
    DBG2_PRINT("save & play = >>",play,save);
}

__attribute__((weak)) void os_set_volume_resume()
{
    DBG_PRINT("get volume resume >>",0);
}

__attribute__((weak)) int os_get_volume()
{
    DBG_PRINT("get volume >>",0);
    return 0;
}


//配置网络
/*
-1:改变当前状态
0：退出连接状态
1: 进入连接状态
*/

__attribute__((weak)) void os_set_net_config_mode(int mode)
{
    DBG_PRINT("os_set_net_config_mode",mode);
}


__attribute__((weak)) int os_get_net_config_mode()
{
    DBG_PRINT("os_get_net_config_mode",0);
    return 0;

}

__attribute__((weak)) int os_set_net_config_info(char *ssid,char *pwd)
{
    STRDBG_PRINT("set net ssid >>",ssid);
    STRDBG_PRINT("set net pwd >>",pwd);
    return 0;
}


__attribute__((weak)) int os_get_net_connect()
{
    return 0;
}

__attribute__((weak)) int os_get_bt_ops_status()
{
    return 0;
}

//
__attribute__((weak)) int os_set_delay_ms(int ms)
{
    mdelay(ms);
    return 0;
}

__attribute__((weak)) int os_set_delay_us(int ms)
{
    udelay(ms);
    return 0;
}

__attribute__((weak)) uint32_t os_get_time_ms()
{

    return timer_get_ms();;
}


__attribute__((weak)) void os_set_bluetooth_connect()
{
    robot_control_preset_do(PRESET_BT_OK);
}

__attribute__((weak)) void os_set_bluetooth_disconnect()
{
    robot_control_preset_do(PRESET_BT_DISOK);
}

__attribute__((weak)) void os_set_wifi_connect()
{
    robot_control_preset_do(PRESET_WIFI_OK);
}

__attribute__((weak)) void os_set_wifi_disconnect()
{
    robot_control_preset_do(PRESET_WIFI_DISOK);
}

__attribute__((weak)) void os_set_app_connect()
{
    robot_control_preset_do(PRESET_APP_OK);
}

__attribute__((weak)) void os_set_app_disconnect()
{
    robot_control_preset_do(PRESET_APP_DISOK);
}

static int interactive_mode = 0;
__attribute__((weak)) void llm_set_interactive_mode(int mode)
{
    DBG_PRINTF("llm_set_interactive_mode %d",mode);
    interactive_mode = mode;
}

__attribute__((weak)) int llm_get_interactive_mode(int mode)
{
    DBG_PRINTF("llm_get_interactive_mode %d",interactive_mode);
    return interactive_mode;
}


