

#include "robot_main.h"
#include "robot_debug.h"
#include "robot_control.h"
#include "robot_adapter.h"
#include "robot_action.h"
#include "event/key_event.h"
#include "robot_config.h"
#include "robot_app.h"

#define ROBOT_CTL_FLAG_ALLCLR {ctype = zerof = stepf = endf = errf = aid = sid = 0; cs = 0xff;}
#define ROBOT_CTL_FLAG_LINECLR {cs = 0xff; sid = 0; stepf = 2;}


#define CCODE_START (0xF6)
#define CCODE_END   (0xFE)
#define CCODE_SZ    (CCODE_END-CCODE_START+1)

//F5:GOKEEP F6:GOSTOP
//0xF5
//#define CSAVE (0xF5-CCODE_START+1)
//0xF6
#define I2CTS (0xF6-CCODE_START+1)
//0xF7
#define CTSMO (0xF7-CCODE_START+1)
//0xF8
#define CLOAD (0xF8-CCODE_START+1)
//0xF9
#define CINST (0xF9-CCODE_START+1)
//0xFA
#define CCONN (0xFA-CCODE_START+1)
//0xFB
#define CACTN (0xFB-CCODE_START+1)
//0xFC
#define CREAD (0xFC-CCODE_START+1)
//0xFD
#define CLOCK (0xFD-CCODE_START+1)
//0xFE
//#define CUNLO (0xFE-CCODE_START+1)



#define RXBUFSIZE   (4096)
#define RXMASK (0x0FFF)
//auto m
uint16_t zerof = 0;
uint16_t stepf = 0;
uint16_t endf = 0;
uint16_t errf = 0;
uint16_t sendf = 0;
uint16_t ctype = 0;
uint8_t cs = 0xff;

uint8_t adata[200][20];
uint8_t sdata[15];
uint16_t rdata[20];
uint8_t sbuff[20];//2+17+1;
uint16_t aid=0,sid=0;

int tbz_hs_flag;
int tbz_rtc_mode;
int tbz_vol = 2;
int tbz_power;
int tbz_power_call_judge;
int tbz_power_call_set;
int tbz_att_sw;
int tbz_md_sw;
int tbz_rwup_sw;
int tbz_gscf_sw;
int tbz_wifi_st = 0;
int device_id_lenth = 0;

extern int TbzNetConfigFlag;
int tbzble_setting_ask = 0;

uint8_t g_u8RecData[RXBUFSIZE]  = {0};

uint32_t g_u32comRbytes = 0;
uint32_t g_u32comRhead  = 0;
uint32_t g_u32comRtail  = 0;



//data queue
void robot_control_data_enqueue(uint8_t data[],int sz)
{
    int i;
    for(i=0;i<sz;i++)
    {
        if(g_u32comRbytes < RXBUFSIZE)
        {
            g_u8RecData[g_u32comRtail] = data[i];
            g_u32comRtail++;
            g_u32comRtail&= RXMASK;
            g_u32comRbytes++;
        }
    }
}

void robot_control_clean_enqueue()
{
    const int8_t CLEAN_CMD[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    const int CLEAN_CM_SZ = 6;
    robot_control_data_enqueue(CLEAN_CMD,CLEAN_CM_SZ);
}


int robot_control_data_queuesize()
{
    return g_u32comRbytes;
}

int robot_control_data_dequeue()
{
    int ret = g_u8RecData[g_u32comRhead];
    g_u32comRhead++;
    g_u32comRhead &= RXMASK;
    g_u32comRbytes--;
    return ret;
}

void tbz_app_setting_info_send(int appsend)
{
    /*
    int i;
    uint8_t data[20];
    int dsz = 0;
    data[dsz++] = 0xF4;
    data[dsz++] = 0x4F;
    data[dsz++] = 0x07;
    data[dsz++] = GetAboteVolume();
    data[dsz++] = 2-ABOTE_SWITCH_CMDASR;
    data[dsz++] = 2-ABOTE_SWITCH_MUSICDANCE;
    data[dsz++] = (ABOTE_SWITCH_GSCMODE)?(1):(2);
    data[dsz++] = robot_vbat_app_level_get();
    data[dsz++] = GetWiFiState();
    data[dsz++] = 2-ABOTE_SWITCH_ATT;
    if(appsend)
    {
        APP_SEND(data,dsz);
    }
    else
    {
        for(i=3;i<dsz;i++)
            DBG2_PRINT("ABOTE_info >> ",i,data[i]);
    }
    */
}

int app_wifi_send_flag = 0;
void app_wifistatus_send(int wifiok)
{
    if(app_wifi_send_flag)
    {
        uint8_t data[10];
        int dsz = 0;
        app_wifi_send_flag = 0;
        data[dsz++] = 0xF4;
        data[dsz++] = 0x4F;
        data[dsz++] = 0x07;
        data[dsz++] = 0x05;
        data[dsz++] = wifiok;
        APP_SEND(data,dsz);
    }
}


char deviceid[64];
void app_online_info_send()
{
    int i;
    int lenidx;
    uint8_t data[20];
    int dsz = 0;
    app_wifi_send_flag = 0;
    data[dsz++] = 0xF4;
    data[dsz++] = 0x4F;
    data[dsz++] = 0x06;
    if(!os_get_net_connect())
    {
        data[dsz++] = 0x00;
    }
    else
    {
        get_device_id(deviceid);
        lenidx = dsz;
        data[dsz++] = 0;
        for(i=0;deviceid[i];i++)
            data[dsz++] = deviceid[i];
        data[lenidx] = i;
    }
    APP_SEND(data,dsz);
}

void app_mac_info_send()
{
    int i;
    int lenidx;
    uint8_t data[20];
    int dsz = 0;
    app_wifi_send_flag = 0;
    data[dsz++] = 0xF4;
    data[dsz++] = 0x4F;
    data[dsz++] = 0x06;
    if(!os_get_net_connect())
    {
        data[dsz++] = 0x00;
    }
    else
    {
        extern const unsigned char *bt_get_mac_addr(void);
        unsigned char* mac = bt_get_mac_addr();
        data[dsz++] = 0x06;
        for(i=0;i<6;i++)
            data[dsz++] = mac[i];
    }
    APP_SEND(data,dsz);
}

void robot_control_prompt_make(int cmd1,int cmd2)
{
    switch(cmd1)
    {
    case 0x01://音量
        switch(cmd2)
        {
        case 1:os_set_play_audio("VolumeEmpty.mp3",NULL);break;
        case 2:os_set_play_audio("VolumeFull.mp3",NULL);break;
        default:os_set_play_audio("Volume.mp3",NULL);break;
        }
        break;
    case 0x02://App
    case 0x03://WiFi
        break;
    }
}


int robot_command_setting_data_make(int setitem,int setdata)
{
    switch(setitem)
    {
        case 0x01:tbz_app_setting_info_send(1);break;
        case 0x02:
            tbz_app_setting_volume_set(sdata[2]);
            if(sdata[2] == 1) {
                robot_control_prompt_make(0x01, 1); // 音量为空
            } else if(sdata[2] == 5) {
                robot_control_prompt_make(0x01, 2); // 音量为满
            } else {
                robot_control_prompt_make(0x01, 0); // 正常音量
            }
        break;
        case 0x03:Tbz_AsrSwitchSet(2-sdata[2]);
            break;
        case 0x04:Tbz_musicdance_switch_set(2-sdata[2],1,1);
            break;
        case 0x05:
            if(sdata[2]==0x05)
            {
                DBG_PRINT("WIFI NG",0);
                sdata[2]=0x01;
            }
            if(sdata[2]==0x03)
            {
                app_wifi_send_flag = 1;
                    //ex_clear_turing_net_cfg_info();
            }else if(sdata[2]==0x01)
            {
                STR_PRINT("IN WIFI GO!\r\n");
                if(!is_in_config_network_state())
                {
                    robot_asr_switch_set(2);
                    os_set_net_config_mode(-1);
                }
                //ex_clear_turing_net_cfg_info();
            }else if(sdata[2]==0x02)
            {
                if(is_in_config_network_state())
                {
                    //os_set_net_mode();
                    robot_asr_switch_set(3);
                }
                //ex_clear_turing_net_cfg_info();
            }
            break;
            case 0x06:
                Tbz_GSCModeSet(2-sdata[2],1,1);
            break;
            case 0x07:
                Tbz_AttModeSet(2-sdata[2],1,1);
            break;
    }
}



void robot_control_light_make(int cmd1,int cmd2)
{

}

void robot_control_action_make(int cmd1,int cmd2)
{

}



void robot_remote_action_make(uint8_t data[])
{
    switch(data[0])
    {
    case 1:
        break;
    case 2:
        robot_action_command_play(data[1],data[2],data[3]);
        break;
    }

}



int robot_command_data_make()
{
    switch(sdata[0])
    {
    case 0x04://App编程
        robot_program_command_make(sdata[1],&sdata[2]);
        break;
    case 0x05://App遥控
        //robot_remote_action_make(&sdata[1]);
        //robot_program_touchset(1);
        break;
    case 0x06://App在线内容
        if(sdata[1]==0x01)
        {
            app_online_info_send();
        }
        if(sdata[1]==0x02)
        {
            app_mac_info_send();
        }
        break;
    case 0x07://App设置
        robot_command_setting_data_make(sdata[1],sdata[2]);
        break;
    case 0x08://群演
        Abote_GroupShow_AppData_Make(&sdata[1]);
        break;
    case 0x11://提示音
        robot_control_prompt_make(sdata[1],sdata[2]);
        break;
    case 0x12://灯光
        robot_control_light_make(sdata[1],sdata[2]);
        break;
    case 0x13://动作
        robot_control_action_make(sdata[1],sdata[2]);
        break;

    }
    return 0;
}


//control
int robot_control_data_once()
{
    //DBG_PRINT("g_u32comRbytes = ",g_u32comRbytes);22
    while(robot_control_data_queuesize())
    {
        uint8_t u8InChar = robot_control_data_dequeue();
        if(u8InChar==0xff)
        {
            zerof++;
            if(zerof>=5)//clean
            {
                ROBOT_CTL_FLAG_ALLCLR;
                STR_PRINT("all flag zero!\r\n");
                continue;
            }
        }
        else
        {
            zerof = zerof>>1;
        }
        if(errf)
            continue;
        switch(stepf)
        {
            case 2:
                if((ctype)&&(ctype<=CCONN))
                    sdata[sid++] = u8InChar;
                else
                    adata[aid][sid++] = u8InChar;
                cs ^= u8InChar;
                if(sid == (((ctype)&&(ctype<=CCONN))?(COMMAND_DATA_SZ):(ACTION_DATA_SZ)))
                {
                    if(cs)//last data check sum
                    {
                        int i;
                        DBG_PRINT("aid = ",aid);
                        DBG_PRINT("cs = ",cs);
                        for(i=0;i<sid;i++)
                            DBG_PRINT("adata = ",adata[aid][i]);
                        errf = 1;
                    }
                    else
                    {

                        aid++;
                        sendf = endf;
                        stepf = 0;
                    }
                }
                break;

            case 0:
                if(u8InChar!=0xff)
                {
                    if((u8InChar >= CCODE_START)&&(u8InChar <= CCODE_END))//last f
                    {
                        ROBOT_CTL_FLAG_LINECLR;
                        ctype = 1 + (u8InChar - CCODE_START);
                        endf = 1;
                        if((aid == 0)&&(ctype>CCONN))
                        {
                            memset(adata,0,sizeof(adata));
                        }
                    }
                    else if(u8InChar == aid)
                    {
                        ROBOT_CTL_FLAG_LINECLR;
                        if(aid == 0)
                        {
                            memset(adata,0,sizeof(adata));
                        }
                    }
                    else//err
                    {
                        DBG_PRINT("head err u8InChar = ",u8InChar);
                        errf = 2;
                        stepf = 0xFF;
                    }
                }
                break;
            default:
                break;
        }
    }
    //DBG_PRINT("sendf = ",sendf);
    if(sendf)
    {
        DBG_PRINT("Data Send Type = ", ctype);
        switch(ctype)
        {
            case CINST : robot_main_input_mode_set(sdata[0]);break;
            case CCONN : robot_command_data_make();break;
            case CACTN : /*robot_action_data_make();*/robot_action_th01_data_make();break;
            //case CSAVE : R11_SaveInfoGet();R11_ActionAdjust();R11_ADATA_LOAD();AC79_Mode_Send(R11_IDLE);break;
            case CREAD : robot_action_ts15_posi_read();break;
            case CLOCK : robot_action_ts15_lock();break;
            case CLOAD : robot_action_data_saveplay(sdata[0],sdata[1]);break;
            //case CTSMO : TS15_ModeSet();break;
            //case I2CTS : R11_I2C_Test();break;
        }
        sendf = 0;
        ROBOT_CTL_FLAG_ALLCLR;
    }
    if(errf)
    {
        if(errf!=0x10)
        {
            DBG_PRINT("Err errf = ", errf);
            errf = 0x10;
        }

    }
    //tbz_net_show_make();
    extern int asr_action_flag;
    if(asr_action_flag==1)
    {
        asr_action_flag = 2;
    }
}

int robot_control_once()
{
    robot_control_data_once();
}

int robot_control_communication_checksum(uint8_t *data,int sz)
{
    int i;
    uint8_t cs=0xFF;
    for(i=1;i<sz;i++)
        cs ^= data[i];
    return cs;
}


void robot_control_command_send(uint8_t tag,uint8_t *data,int sz)
{
    int i;
    uint8_t cmd[COMMAND_DATA_SZ+1];
    cmd[0] = tag;
    for(i=1;i<=COMMAND_DATA_SZ-1;i++)
    {
        if(i<=sz)
            cmd[i] = data[i-1];
        else
            cmd[i] = 0;
    }
    cmd[COMMAND_DATA_SZ] = CTRL_CHECKSUM(cmd,COMMAND_DATA_SZ);
    CTRL_CLEAN_SEND();
    CTRL_SEND(cmd,COMMAND_DATA_SZ+1);
    HEXS_PRINT("tbz_app_command_send data = ",cmd,COMMAND_DATA_SZ);
}

void robot_control_volumeshow_send(int vol)
{
    uint8_t data[2];
    data[0] = 0x01;
    data[1] = vol;//get_volume();
    robot_control_command_send(0xFA,data,2);

}

void robot_control_audio_play_send(int idx)
{
    uint8_t data[2];
    data[0] = 3 ;
    data[1] = idx;
    robot_control_command_send(0xF8,data,2);
}



extern int face_data_mid;
void robot_control_action_open_make()
{
    int i;
    extern uint16_t ts15_data_init[];
    adata[0][0] = 10;
    adata[0][1] = 10;
    for(i=2;i<=19;i++)
    {
        if(i<=10)
        {
            adata[0][i] = ts15_data_init[i-1];
            //if((i==7)||(i==8))
            //    adata[0][i] += ts15_data_dir[i-1]*face_data_mid;
        }
        else
        {
            adata[0][i] = 0;
        }
    }
    aid = 1;
    //DBG2_PRINT("open = ",adata[0][0],adata[0][1]);
    robot_action_th01_data_make();
}

void robot_control_preset_send(int idx)
{
    uint8_t data[2];
    data[0] = 3 ;
    data[1] = idx;
    robot_control_command_send(0xF8,data,2);
}


void robot_control_preset_do(int idx)
{
    DBG_PRINT("control preset do",idx);
    switch(idx)
    {
    case PRESET_OPEN:
        break;
    case PRESET_WIFI_OK:
        break;
    case PRESET_WIFI_DISOK:
        break;
    case PRESET_BT_OK:
        os_set_play_audio("btok.mp3",NULL);
        break;
    case PRESET_BT_DISOK:
        os_set_play_audio("btng.mp3",NULL);
        break;
    case PRESET_APP_OK:
        os_set_play_audio("appok.mp3",NULL);
        break;
    case PRESET_APP_DISOK:
        os_set_play_audio("appng.mp3",NULL);
        break;
    }

}
