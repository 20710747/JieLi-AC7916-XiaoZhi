#include "robot_main.h"
#include "robot_debug.h"
#include "robot_adapter.h"
#include "robot_config.h"
//#include "robot_app.h"

#define CFG_SAORAGE_PAGE_ADDR (321)

#define CFG_DATA_SZ 15
uint8_t cfgdata[CFG_DATA_SZ]=
{
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
};

int Config_Init = 0;

//ROBOT_CONFIG_ENABLE
#define ABOTE_CONFIG_ENABLE 0
#define ABOTE_CONFIG_DISABLE 1
#define ABOTE_DEFAULT_ASRTYPE 2

//ROBOT_SWITCH_CMDASR
/*
int ABOTE_SWITCH_CMDASR = 0;
int ABOTE_SWITCH_ASRTYPE = 0;
int ABOTE_SWITCH_TOUCH = 0;
int ABOTE_SWITCH_AUTOGETUP = 0;
int ABOTE_SWITCH_ATT = 0;
int ABOTE_SWITCH_DISTANCE = 0;
int ABOTE_SWITCH_GYD = 0;
int ABOTE_SWITCH_DEBUG = 0;
int ABOTE_SWITCH_BATTERY = 0;
int ABOTE_SWITCH_MUSICDANCE = 0;
int ABOTE_SWITCH_VOICEMODE = 0;
int ABOTE_SWITCH_GSCMODE = 0;
int ABOTE_DATA_VOLUME = 0;
int ABOTE_OPEN_IDX = 0;
int ABOTE_UPDATE_AGREE = 0;
int ABOTE_TEST_MODE = 0;
int ABOTE_TURING_NEWFLAG = 0;
*/

int ROBOT_SWITCH_ASRWAKEUP = 0;
int ROBOT_DATA_LANG = 0;
int ROBOT_DATA_VCN = 0;
int ROBOT_DATA_PERSONA = 0;
int ROBOT_DATA_VOLUME = 0;
//int ROBOT_DATA_DEBUG = 0;
int debug_mode = 0;
void robot_debug_set_mode(int mode)
{
    /*
    switch(mode)
    {
    case 5:
        mode = ABOTE_SWITCH_DEBUG;
        break;
    default:
        debug_mode = mode;
        if(mode)
            DBG_PRINT("debug_mode = ",mode);
        Tbz_Config_Save(6,mode);
    }
    */

}

//robot_config_read
void Tbz_Config_Read(int debug)
{
    int i;
    if(robot_storage_adata_read(CFG_SAORAGE_PAGE_ADDR)>=1)
    {
        robot_storage_adata_get((uint8_t*)cfgdata, CFG_DATA_SZ, 0);
    }
    else
    {
        DBG_PRINT("no sava data , use default",-1);
        cfgdata[0] = 1;
        cfgdata[1] = 1;
        cfgdata[2] = 1;
        cfgdata[3] = 0;
        cfgdata[4] = 8;
    }

    ROBOT_SWITCH_ASRWAKEUP = cfgdata[0];
    ROBOT_DATA_LANG = cfgdata[1];
    ROBOT_DATA_VCN = cfgdata[2];
    ROBOT_DATA_PERSONA = cfgdata[3];
    ROBOT_DATA_VOLUME = cfgdata[4];
/*
    ABOTE_SWITCH_CMDASR = cfgdata[0] == ABOTE_CONFIG_ENABLE;
    ABOTE_SWITCH_AUTOGETUP = cfgdata[1] == ABOTE_CONFIG_ENABLE;
    ABOTE_SWITCH_TOUCH = cfgdata[2] == ABOTE_CONFIG_ENABLE;
    ABOTE_SWITCH_ATT = cfgdata[3] == ABOTE_CONFIG_ENABLE;
    ABOTE_SWITCH_DISTANCE = cfgdata[4] == ABOTE_CONFIG_ENABLE;
    ABOTE_SWITCH_GYD = cfgdata[5] == ABOTE_CONFIG_ENABLE;
    ABOTE_SWITCH_DEBUG = cfgdata[6];
    ABOTE_SWITCH_ASRTYPE = cfgdata[7] == 0 ? ABOTE_DEFAULT_ASRTYPE : cfgdata[5];
    ABOTE_SWITCH_BATTERY = cfgdata[8] == ABOTE_CONFIG_ENABLE;
    ABOTE_SWITCH_MUSICDANCE = cfgdata[9];
    ABOTE_SWITCH_VOICEMODE = cfgdata[10];
    ABOTE_SWITCH_GSCMODE = cfgdata[11];
    ABOTE_DATA_VOLUME = cfgdata[12];
    ABOTE_OPEN_IDX = cfgdata[13];
    ABOTE_UPDATE_AGREE = cfgdata[14];
    ABOTE_TEST_MODE = cfgdata[15];
    ABOTE_TURING_NEWFLAG = cfgdata[16];
*/
#if 1
    if(debug)
    {
        DBG_PRINT("[0]asr_onoff<0:off 1:on> = ",cfgdata[0]);
        DBG_PRINT("[1]language<1:zhcn 2:enus> = ",cfgdata[1]);
        DBG_PRINT("[2]vcn<1:male 2:female 3,4:child> = ",cfgdata[2]);
        DBG_PRINT("[3]persona<0:off 1:on> = ",cfgdata[3]);
        DBG_PRINT("[4]volume<1-5> = ",cfgdata[4]);
    }
#endif // 0
}

//robot_config_change
int Tbz_Config_Change(int cfg, int data)
{
    //if (cfg == 5)
    //    return data == 0 ? ABOTE_DEFAULT_ASRTYPE : data;
    //else if (cfg == 0 || cfg == 1 || cfg == 2 || cfg == 3 || cfg == 4|| cfg == 5|| cfg == 8)
    //    return data ? ABOTE_CONFIG_ENABLE : ABOTE_CONFIG_DISABLE;
    //else
    //    return data;
    return data;
}

//robot_config_save
void Tbz_Config_Save(int cfg, int data)
{

    if(!Config_Init)
    {
        DBG_PRINT("Tbz_Config_Save No Init",-1);
        return;
    }

    DBG2_PRINT("Tbz_Config_Save",cfg,data);
    cfgdata[cfg] = Tbz_Config_Change(cfg, data);
    DBG2_PRINT("CFG&DATA = ",cfg,data);
    switch(cfg)
    {
    case 0:
        ROBOT_SWITCH_ASRWAKEUP = cfgdata[0];
        break;
    case 1:
        ROBOT_DATA_LANG = cfgdata[1];
        break;
    case 2:
        ROBOT_DATA_VCN = cfgdata[2];
        break;
    case 3:
        ROBOT_DATA_PERSONA = cfgdata[3];
        break;
    case 4:
        ROBOT_DATA_VOLUME = cfgdata[4];
        break;
    }
    robot_storage_adata_set(cfgdata, CFG_DATA_SZ, 0);
    robot_storage_adata_write(CFG_SAORAGE_PAGE_ADDR, 1);
    DBG_PRINT("Config Data Save OK", 1);
    Tbz_Config_Read(1);
}

//robot_config_clean
void Tbz_Config_Clean()
{
    int i;
    for(i=0;i<CFG_DATA_SZ;i++)
        cfgdata[i] = 0;
    robot_storage_adata_set(cfgdata,CFG_DATA_SZ,0);
    robot_storage_adata_write(CFG_SAORAGE_PAGE_ADDR,1);
}


int tbz_update_agree()
{
    //return ABOTE_UPDATE_AGREE;
    return 0;
}

void tbz_set_update_once()
{
    //Tbz_Config_Save(12,1);
}

void tbz_set_update_clr()
{
    //Tbz_Config_Save(12,0);
}

int robot_config_turing_newflag()
{
    //return (ABOTE_TURING_NEWFLAG==0);
    return 0;
}

void robot_config_turing_delnew()
{
    Tbz_Config_Save(14,1);
}


void robot_config_turing_setnew()
{
    Tbz_Config_Save(14,0);
}

extern void Tbz_Config_Read(int debug);
extern char deviceid[64];

void Tbz_ABOTE_Info_Debug()
{
    u8 para[4];
    u8 *mac;
    extern const char *bt_get_local_name(void);
    extern const u8 *bt_get_mac_addr(void);

    //robot_debug_set_mode(6);
    STRDBG_PRINT("DEVICE:",bt_get_local_name());
    STRDBG_PRINT(TBZ_ABOTE_VERSION_TAG,TBZ_TURNING_VERSION);
    get_device_id(deviceid);
    if(strlen(deviceid)==0)
        STRDBG_PRINT("turing_devid : ","no connect!");
    else
        STRDBG_PRINT("turing_devid : ",deviceid);
    mac = bt_get_mac_addr();
    HEXS_PRINT("mac",mac,6);
    STRDBG_PRINT("ABOTE_CFG:",">>");
    Tbz_Config_Read(1);
    robot_debug_set_mode(5);
}

void Tbz_ABOTE_Info_App()
{
    uint8_t data[20];
    int dsz = 0;
    data[dsz++] = 0xF4;
    data[dsz++] = 0x4F;
    data[dsz++] = 0x0D;
    data[dsz++] = 0xFF;
    data[dsz++] = TBZ_ABOTE_VERSION_A;
    data[dsz++] = TBZ_ABOTE_VERSION_B;
    data[dsz++] = TBZ_ABOTE_VERSION_C;
    extern uint16_t robot_app_data_send(uint8_t  * data, uint16_t sz);
    robot_app_data_send(data,dsz);
}

void Tbz_ABOTE_Info_ShowVersion()
{
    action_delay(200);
    Tbz_ABOTE_Info_ShowNumber(TBZ_ABOTE_VERSION_A);
    Tbz_ABOTE_Info_ShowNumber(TBZ_ABOTE_VERSION_B);
    Tbz_ABOTE_Info_ShowNumber(TBZ_ABOTE_VERSION_C);
}

void Tbz_ABOTE_Info()
{
    Tbz_ABOTE_Info_App();
    Tbz_ABOTE_Info_Debug();
    //Tbz_ABOTE_Info_ShowVersion();
}

void robot_config_init()
{
    Config_Init = 0;
    Tbz_Config_Read(1);
    //ABOTE_SWITCH_MUSICDANCE = 0;
    //extern void Tbz_musicdance_switch_set(int set,int save,int play);
    //Tbz_musicdance_switch_set(ABOTE_SWITCH_MUSICDANCE,0,0);
    //tbz_dac_set_volume(ABOTE_DATA_VOLUME,0,0);
    //tbz_dac_resume_volume();
    Config_Init = 1;
}

#if 0
int next_asr_switch = -1;

void Tbz_AsrSwitchDo()
{
    robot_main_debug_output("next_asr_switch = ",next_asr_switch);
    if(next_asr_switch!=-1)
    {
        if(next_asr_switch==1)
            app_music_play_voice_prompt("asron.mp3", NULL);
        else
            app_music_play_voice_prompt("asroff.mp3", NULL);
        robot_asr_switch_set(next_asr_switch);
    }
    next_asr_switch = -1;
}

int Tbz_AsrSwitchSet(int sw)
{
    next_asr_switch = sw;
    Tbz_AsrSwitchDo();
}
#endif
int tbz_gscmode = 0;

void Tbz_GSCModeSet(int mode,int save,int play)
{
    tbz_gscmode = mode;
    uint8_t sdata[2];
    sdata[0] = 0x02;
    sdata[1] = tbz_gscmode;
    if(save)
        Tbz_Config_Save(11,mode);
    if(play)
        Abote_GroupShow_AppData_Make(sdata);
}

int att_mode = -1;
void Tbz_AttModeSet(int mode,int save,int play)
{
    att_mode = mode;
    DBG_PRINT("att_mode",att_mode);
    if(att_mode == 1)
      os_set_play_audio("attitu_o.mp3",NULL);
    else
      os_set_play_audio("attitu_c.mp3",NULL);
    if(save)
        Tbz_Config_Save(3,mode);
    if(play)
        robot_posi_data_once(1,3);
}

