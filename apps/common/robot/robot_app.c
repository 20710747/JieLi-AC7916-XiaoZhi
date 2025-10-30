#include "robot_main.h"
#include "robot_debug.h"
#include "robot_control.h"
#include "robot_adapter.h"
#include "robot_app.h"
//#include "robot_action.h"
//#include "robot_config.h"



int AppDebugMode = 0;
//int AppRemoteDir = 0;
//int AppRemoteKeep = 0;
void robot_app_debug_mode_set(int set)
{
    AppDebugMode = set;
    DBG_PRINT("AppDebugMode = ",set);
}

void robot_app_debug_mode_switch()
{
    robot_app_debug_mode_set(1-AppDebugMode);
}

uint16_t robot_app_data_check(uint8_t *data,uint8_t len)
{
	const uint8_t check_data[6] = {'T','B','T','H','0','1'};//"TBZR12"
	DBG_PRINT("robot_app_data_check sz = ",len);
	uint16_t i,ret = 0;
    if(AppDebugMode)
        HEXS_PRINT("APP_RECV >> ",data,len);
        //robot_app_data_show(data,len,"APP_RECV >> ");
	if(len<6)
        return 0;
	for(i=0;i<6;i++)
	{
		if(data[i]!=check_data[i])
            break;
	}
	if(i>=6)
		ret = 1;
    DBG_PRINT("robot_app_data_check ret = ",ret);
	return ret;
}

void tbz_app_action_send(uint8_t *data,int sz)
{
    int i;
    uint8_t cmd[ACTION_DATA_SZ+1];
    if(sz<ACTION_DATA_SZ)
        return ;
    for(i=0;i<ACTION_DATA_SZ;i++)
        cmd[i] = data[i];
    //DBG_PRINT("tbz_app_action_send head = ",cmd[0]);
    cmd[ACTION_DATA_SZ] = CTRL_CHECKSUM(cmd,ACTION_DATA_SZ);
    CTRL_SEND(cmd,ACTION_DATA_SZ+1);
}
int robot_control_command_holdcmd(int flag, uint8_t *data) {
    return 1;
}
//robot_app_command_send();
extern int  robot_action_command_advance_remoteflag;
void tbz_app_command_send(uint8_t tag,uint8_t *data,int sz)
{
    if(robot_action_command_advance_remoteflag)
    {
        if(robot_control_command_holdcmd(robot_action_command_advance_remoteflag,data))
            robot_action_command_advance_hold_set(1);
    }
    else
    {
        robot_control_command_send(tag,data,sz);
    }

}


int robot_app_code_get(uint8_t *data)
{
    int code = -1;
    if((data[0]>=0xF0)&&(data[0]<=0xFF))
    {
        code = 0xFF - data[0];
        if(data[1]+code*0x10!=0xFF)
            code = -1;
    }
    if(code!=-1)
    {
        DBG_PRINT("robot_app_code = ",code);
    }
    return code;
}


void robot_app_entrance(uint8_t *data,int sz)
{
    int app_code = robot_app_code_get(data);
    if(app_code<0)
    {
        //tbz_app_command_send(data,sz);
    }
    else
    {
        switch(app_code)
        {
        case 0:CTRL_CLEAN_SEND();break;//FFFF
        case 1:STR_PRINT("App Connect!!");robot_control_audio_play_send(7);break;//FEEF
        case 2:tbz_app_action_send(data+2,sz-2);break;//FDDF
        case 3:tbz_app_command_send(0xFA,data+2,sz-2);break;//FCCF
        case 8:STR_PRINT("Action Stop\r\n");break;//F77F
        case 9:robot_remote_keep_set(0);break;//F66F
        case 10:robot_remote_keep_set(1);break;//F55F
        case 14:robot_main_input_mode_set(MAININPUT_MODE_CMD);break;//F1 1F
        case 15:tbz_app_command_send(data[2],data+3,sz-3);//F00F
        default:
            break;
        }
    }
}

#if 0
void tbz_net_show_make()
{

    extern int net_connect_show;
    extern int net_disconnect_show;
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
#endif

extern int turing_ble_user_data_send(unsigned char  * data, int sz);

uint16_t robot_app_data_send(uint8_t  * data, uint16_t sz)
{
    //if(AppDebugMode)
    //    HEXS_PRINT("APP_SEND << ",data,sz);
    //return turing_ble_user_data_send(data,sz);
    return 0;
}

void tbz_app_setting_volume_set(int volume)
{
    DBG_PRINT("volume set ",volume);
    os_set_volume(volume,1,1);
}

#if 0
    switch(data[0])
    {
    case 0x02://连接
        if(data[1]==0x01)
        {
            tbzble_handshake_touch();
            printf("app connect!!");
        }
        break;
    case 0x03://状态
        tbzrtc_set_mode(data[1]);
        if(data[1]==3)
        {
            tbz_idle_set();
            app_programmer_flag_set(1);
            tbz_programme_set();
        }
        else
        {
            tbz_idle_set();
            app_programmer_flag_set(0);
        }
        if(data[1]==1)
        {
            //uart2p4g_data_send("hello",5);
            //GroupShow_Set_Main();
        }
        else if(data[1]==0)
        {
            GroupShow_Set_Idle();
        }

        break;
    case 0x04://遥控
        //指令转发，这里不作处理
        //触摸关掉
        if(data[1]==0x07)
        {
             //TTP_Data_test(1);
             //playmusicstop();
             //head_touch_flag = 0;
             app_music_exit_old_mode(0);
             head_touch_flag = 1;
             //sys_timeout_add(NULL,tbz_touchflag_clr,2000);
        }
        else
        {
             head_touch_flag = 1;
        }
        if(data[1]==0x51)
        {
            head_touch_flag = 0;
            GroupShow_Set_Main();
        }
        else if(data[1]==0x52)
        {
            head_touch_flag = 0;
            GroupShow_Set_Idle();
        }
        else
        {
            GroupShow_Action_Send(data[1]);
        }
        break;
    case 0x05://编程
        tbz_programme_set();
        tbz_programme_deal(data+1,sz-1);
        break;
    case 0x06://动作
        break;
    case 0x07://设置
        {
            switch(data[1]&0xF0)
            {
            case 0x00:
                if(data[1]==0x01)
                {
                    tbz_power = app_battery_get(app_battery_vlaue);//获取电量
                    tbz_vol = get_app_music_volume()/5;
                    tbz_wifi_st = network_connect_check();
                    tbzble_setting_ask = 1;
                    //tbzble_setting_send();
                }
                break;
            case 0x10:
                tbz_vol = data[1]&0x0F;
                app_volume_set(tbz_vol*5);
                printf("tbz set vol : %d",tbz_vol);
                break;
            case 0x30:
                tbz_att_sw = data[1]&0x0F;
                printf("tbz set att_sw : %d",tbz_att_sw);
                att_voice_play(tbz_att_sw);
                break;
            case 0x50:
                tbz_md_sw = data[1]&0x0F;
                printf("tbz set music_dance : %d",tbz_md_sw);
                musicdance_voice_play(tbz_md_sw);
                break;
            case 0x60:
                tbz_rwup_sw = data[1]&0x0F;
                printf("tbz set asr_wakeup : %d",tbz_rwup_sw);
                asrwakeup_voice_play(tbz_rwup_sw);
                break;
            case 0x70:
                tbz_gscf_sw = data[1]&0x0F;
                printf("tbz set groupshow cflag : %d",tbz_gscf_sw);
                groupshowcf_voice_play(tbz_gscf_sw);
                uart2p4g_showsubflag_set((tbz_gscf_sw==1)?(1):(0));
                break;
            case 0x40:
                if(data[1]==0x41)//进入配网
                {
                    if(!is_in_config_network_state())
                    {
                        struct sys_event evt = {0};
                        evt.type = SYS_KEY_EVENT;
                        evt.u.key.event = KEY_EVENT_LONG;
                        evt.u.key.value = KEY_MODE;
                        sys_event_notify(&evt);
                    }
                }
                else if(data[1]==0x42)//退出配网
                {
                    if(is_in_config_network_state())
                    {
                        struct sys_event evt = {0};
                        evt.type = SYS_KEY_EVENT;
                        evt.u.key.event = KEY_EVENT_LONG;
                        evt.u.key.value = KEY_MODE;
                        sys_event_notify(&evt);
                    }
                }
                printf("is_in_config_network_state = %d\n",is_in_config_network_state());
                break;
            }

        }
        break;
    }
#endif
