#include "robot_main.h"
#include "robot_uart.h"
#include "robot_debug.h"
#include "robot_control.h"
#include "robot_app.h"
#include "robot_program.h"
#include "robot_action.h"
#include "robot_key.h"
#include "robot_posi.h"
#include "robot_adapter.h"

#define ROBOT_PROGRAM_EMOJI_MODE 1
#define ROBOT_PROGRAM_LCD_MODE 2

#define ROBOT_PROGRAM_VOICD_STOP_DEFAULT 0
#define ROBOT_PROGRAM_VOICD_STOP_BREAK 1
#define ROBOT_PROGRAM_VOICD_STOP_WAIT 2
#define ROBOT_PROGRAM_VOICD_STOP_IGNORE 3

uint16_t robot_program_flag = 0;
uint16_t robot_program_runflag = 0;
uint16_t robot_program_emojimode = ROBOT_PROGRAM_EMOJI_MODE;
uint8_t robot_program_2p4g_data[5];

uint16_t robot_program_voice_breakmode = ROBOT_PROGRAM_VOICD_STOP_BREAK;
uint16_t robot_program_voice_volume = 3;

uint16_t robot_program_emoji_speed;

static uint8_t times_flag = 1;


extern void robot_program_run_start();
extern void robot_program_run_stop();
extern int os_get_play_status();//1播放 0不播放
//存储与恢复系统音量 0 存储 1恢复
extern int __extern_recovery_sys_volume(int type);

void robot_program_flag_set(int set)
{
    robot_program_flag = set;
    if(set == 0)
    {
        robot_program_run_stop();
    }
}

uint16_t robot_program_flag_get()
{
    return robot_program_flag;
}

void robot_program_runflag_set(int set)
{
    robot_program_runflag = set;
}

uint16_t robot_program_runflag_get()
{
    return robot_program_runflag;
}

void robot_program_emoji_set_mode(uint16_t mode)
{
    DBG_PRINT("robot_program_emoji_set_mode ",mode);
    if(mode != robot_program_emojimode)
    {
        robot_program_emojimode = mode;
        //发码
    }
}


int robot_program_sys_command_make(int sys_com)
{
    int back = 0;
    switch(sys_com)
    {
        case 1:
            DBG_PRINT("ENTER PROGRAM",1);
            robot_program_flag_set(1);
            robot_program_run_stop();
            //__extern_recovery_sys_volume(0);
            extern void robot_program_voice_volume_set(int volume);
            robot_program_voice_volume_set(3);
            RF2P4G_SET_CHANNEL(0,NULL);
            break;//
        case 2:
            DBG_PRINT("EXET PROGRAM",2);
            robot_program_flag_set(0);
            robot_program_run_stop();
            os_set_volume_resume();
            //__extern_recovery_sys_volume(1);
            RF2P4G_SET_CHANNEL(0,NULL);
            break;//
        case 3:
            DBG_PRINT("RUN PROGRAM",3);
            robot_program_runflag_set(1);
            robot_program_run_start();
            RF2P4G_SET_CHANNEL(0,NULL);
            back=5;
            times_flag = 1;
            break;
        case 4:
            DBG_PRINT("STOP PROGRAM",4);
            robot_program_runflag_set(0);
            robot_program_run_stop();
            RF2P4G_SET_CHANNEL(0,NULL);
            break;
        default:
            back = sys_com;
            break;
    }
    return back;
}

void robot_program_action_make(int16_t posi,int16_t action,int16_t times)
{
    robot_action_program_play(posi,action,times);
}
void robot_groupshow_action_make(int16_t posi,int16_t action,int16_t times)
{
    robot_action_groupshow_play(posi,action,times);
}

void robot_program_voice_breakmode_set(int mode)
{
    robot_program_voice_breakmode = mode;
}

//extern int __extern_set_dec_volume(int volume);

void robot_program_voice_volume_set(int volume)
{
    robot_program_voice_volume = volume;
    os_set_volume(robot_program_voice_volume,0,0);
    //__extern_set_dec_volume(volume*10+50);
}
//以mode的方式来打断/等待音乐，如果mode是0则用系统mode
uint16_t robot_program_voice_break_make(int mode)
{
    int ret = 0;
    int wait_time = 0;
    if(mode == 0)
        mode = robot_program_voice_breakmode;
    if(mode == ROBOT_PROGRAM_VOICD_STOP_IGNORE)
        return 1;
    while(1)
    {
        if(!os_get_play_status())
            break;
        if(mode == ROBOT_PROGRAM_VOICD_STOP_BREAK)
        {
            robot_audio_play_break();
            DBG_PRINT("Voice Play Break Try",wait_time++);
        }
        else
        {
            DBG_PRINT("Voice Play Stop Wait",wait_time++);
        }
        delay_2ms(5);
    }

    return ret;
}

void robot_program_voice_make(int16_t type,int16_t voice,uint8_t para[])
{
    int tone,timbre,volume,mode;
    if((type<=3)&&os_get_play_status())
    {
        if(robot_program_voice_break_make(ROBOT_PROGRAM_VOICD_STOP_DEFAULT))
        {
            return ;
        }
    }
    switch(type)
    {
    case 1://音量设置
        volume = voice;
        robot_program_voice_volume_set(volume);
        break;
    case 2://打断设置
        mode = voice;
        robot_program_voice_breakmode_set(mode);
        break;
    case 3://打断当前音乐
        robot_program_voice_break_make(ROBOT_PROGRAM_VOICD_STOP_BREAK);
        break;
    case 4://等待当前音乐结束。
        robot_program_voice_break_make(ROBOT_PROGRAM_VOICD_STOP_WAIT);
        break;
    case 5://音乐声
        robot_audio_music_play(voice);
        break;
    case 6://音符声
        tone = voice;
        timbre = para[0];
        robot_audio_tone_play(tone,timbre);
        break;
    case 7://等待当前音乐结束。
        robot_program_voice_break_make(ROBOT_PROGRAM_VOICD_STOP_WAIT);
        break;
    }
}

void robot_program_asr_make(int16_t type,int16_t voice,int16_t para[])
{
    switch(type)
    {
    case 1:
        //robot_asr_set_recog_once();
        break;
    case 2:
    case 3:
    case 4:
        break;
    }
}

extern void light_ctl_set_rgb(uint16_t mode,uint16_t r4,uint16_t g4,uint16_t b4);
extern void light_ctl_set_col(uint16_t mode,uint16_t colour);

void robot_program_emoji_set_emoji(int type,int emoji)
{
    DBG_PRINT("robot_program_emoji_set_emoji type ",type);
    DBG_PRINT("robot_program_emoji_set_emoji emoji ",emoji);
    robot_lcd_emoji_set(type*20+emoji);


}

void robot_program_emoji_set_speed(int speed)
{
    DBG_PRINT("robot_program_emoji_set_speed",speed);
    robot_program_emoji_speed = speed;
    robot_lcd_emoji_set_speed(speed);
}

void robot_program_emoji_set_playpause(int playpause)
{
    DBG_PRINT("robot_program_emoji_set_playpause",playpause);
    robot_lcd_emoji_play_playpause(playpause);
}

void robot_program_lcd_set_bcolour(int eyeid,int bcolour)
{
    if(eyeid==1)
        DBG_PRINT("robot_program_left_lcd_set_bcolour",bcolour);
    else
        DBG_PRINT("robot_program_rigth_lcd_set_bcolour",bcolour);
    robot_lcd_char_set_bcolour(eyeid,bcolour);
}

void robot_program_lcd_set_ccolour(int eyeid,int ccolour)
{
    if(eyeid==1)
        DBG_PRINT("robot_program_left_lcd_set_ccolour",ccolour);
    else
        DBG_PRINT("robot_program_rigth_lcd_set_ccolour",ccolour);
    robot_lcd_char_set_ccolour(eyeid,ccolour);
}

void robot_program_lcd_clr(int eyeid,int st,int ed)
{
    if(eyeid==1)
    {
        DBG_PRINT("robot_program_left_lcd_clr_start",st);
        DBG_PRINT("robot_program_left_lcd_clr__end",ed);
    }
    else
    {
        DBG_PRINT("robot_program_right_lcd_clr_start",st);
        DBG_PRINT("robot_program_right_lcd_clr__end",ed);
    }
    robot_lcd_char_set_clr(eyeid,st,ed);
}

void robot_program_lcd_set_char(int eyeid,int ccolour)
{
    if(eyeid==1)
        DBG_PRINT("robot_program_left_lcd_set_char",ccolour);
    else
        DBG_PRINT("robot_program_rigth_lcd_set_char",ccolour);
    robot_lcd_char_set_ccolour(eyeid,ccolour);
}

void robot_program_lcd_set_ascii(int lcdidx,int x,int y,uint8_t data)
{
    printf("robot_program_lcd_set_ascii %d\r\n",data);
    robot_lcd_char_set_ascii(data,lcdidx,x,y);
}

void robot_program_lcd_set_asciis(int lcdidx,uint8_t *data)
{
    int x,y,sz;
    int8_t *asciis;
    if(!data)
        return ;
    x = data[0];
    y = data[1];
    sz = data[2];
    asciis = (int8_t*)(&data[3]);
    robot_lcd_char_set_asciis(asciis,sz,lcdidx,x,y);
}

void robot_program_lcd_set_number(int lcdidx,uint8_t *data)
{
    int i,j,k;
    int x,y,len;
    int zeroflag = 0;
    uint8_t data2[8];
    uint8_t temp[3];
    int num;
    if(!data)
        return ;
    x = data[0];
    y = data[1];
    len = data[2];
    num = (data[3]<<8)+(data[4]);
    robot_lcd_char_set_num_int(num,lcdidx,x,y,len,0);
}

void robot_program_emoji_make(int16_t type,int16_t emoji,uint8_t para[])
{
    DBG_PRINT("type",type);
    DBG_PRINT("emoji",emoji);
    int mode;
    int lcd_idx,speed,playpause,colour;
    if(type<=4)
    {
        if(robot_program_emojimode == ROBOT_PROGRAM_LCD_MODE)
        {
            DBG_PRINT("No Emoji Mode",0);
            return ;
        }
    }
    else if(type>=10)
    {
        if(robot_program_emojimode == ROBOT_PROGRAM_EMOJI_MODE)
        {
            DBG_PRINT("No LCD Mode",1);
            return ;
        }
    }
    switch(type)
    {
    case 1://表情模式，屏幕模式
        mode = emoji;
        robot_program_emoji_set_mode(mode);
        break;
    case 2://眼睛表情
        robot_program_emoji_set_emoji(0,emoji);
        break;
    case 3://速度设置
        speed = emoji;
        robot_program_emoji_set_speed(speed);
        break;
    case 4://表情暂停，播放
        playpause = emoji;
        robot_program_emoji_set_playpause(playpause);
        break;
    case 5://背景色设置
        lcd_idx = emoji;
        colour = para[0];
        robot_program_lcd_set_bcolour(emoji,colour);
        break;
    case 6://字体设置
        lcd_idx = emoji;
        colour = para[0];
        robot_program_lcd_set_ccolour(emoji,colour);
        break;
    case 7://字符显示
        lcd_idx = emoji;
        robot_program_lcd_set_asciis(lcd_idx,para);
    case 8://数值显示
        lcd_idx = emoji;
        robot_program_lcd_set_number(lcd_idx,para);
    case 9:
        lcd_idx = emoji;
        robot_program_lcd_clr(lcd_idx,para[0],para[1]);
        break;
    }
}

void robot_program_2p4g(uint8_t data[])
{
    switch(data[0])
    {
    case 1:
        robot_2p4g_switch_set(1);
        break;
    case 2:
        program_rg2p4g_chset(data[1]);
        break;

    case 5:
        extern void robot_program_2p4gdata_clean();
        robot_program_2p4gdata_clean();
        break;
    }
    extern void robot_program_2p4g_make(uint8_t data[]);
    robot_program_2p4g_make(&data[1]);

}

void robot_program_2p4g_make(uint8_t data[])
{
    int i;
    for(i=0;i<4;i++)
    {
        robot_2p4g_program_data_send(i,data[i]);
    }
    robot_2p4g_program_data_send(4,1);//结束码
}

int robot_program_network(uint8_t data[])
{
    int mode = data[0];
    int back = 0;
    switch(mode)
    {
    case 1:
        robot_program_2p4g(&data[1]);
        back = 7;
        break;
    case 2:
        os_set_net_config_mode(data[1]);
        break;
    }
    return back;
}

void robot_program_data_clear(uint8_t para[])
{
    int type = para[0];
    switch(type)
    {
    case 1://避障
        abote_irs = 0;
        break;
    case 2://测距
        abote_distance = 0;
        break;
    case 3://声音强度
        break;
    }
}


void robot_program_external_set(uint8_t para[])
{
    int type = para[0];
    int mode = para[1];
    switch(type)
    {
        case 1://触摸开关
            //robot_program_touchset(mode);
            break;
    }
}

void robot_program_external_make(int16_t type,int16_t cmd,uint8_t para[])
{
    switch(type)
    {
    case 1://激光测距
        robot_program_distance_make(cmd,&para[1]);
        break;
    case 2://头部灯光
        light_ctl_set_col(cmd,para[0]);
        break;
    case 3://数据清除
        robot_program_data_clear(&para[1]);
        break;
    case 4:
        robot_program_external_set(&para[1]);
        break;
    }
}


int robot_program_get_2p4gdata(uint8_t data[])
{
    robot_program_get_robot_2p4gdata(data);
    return 4;
}



int robot_program_get_voice_status(uint8_t *data)
{
    if(!data)
        return 0;
    if(os_get_play_status())
    {
        data[0] = 1;
    }
    else
    {
        data[0] = 0;
    }
    return 2;
}
extern uint16_t robot_program_get_distance(uint8_t*data);

int robot_program_get_external_status(uint8_t data[])
{
    data[0] = robot_program_get_distance(data);
    return 1;
}

extern uint16_t robot_program_get_robot_irdata(uint8_t*data1,uint8_t*data);

int robot_program_get_ir_status(uint8_t data[])
{
    robot_program_get_robot_irdata(data,NULL);
    return 2;
}

int robot_program_get_ir2_status(uint8_t data[])
{
    robot_program_get_robot_irdata(NULL,data);
    return 1;
}

int robot_program_get_robot_2p4gdata(uint8_t *data)
{
    int i;
    for(i=0;i<4;i++)
    {
        if(data)
            data[i] = robot_program_2p4g_data[i];
    }
    return robot_program_2p4g_data[4];

}

void robot_program_2p4gdata_clean()
{
    int i;
    for(i=0;i<5;i++)
        robot_program_2p4g_data[i] = 0;
}


void robot_program_2p4gflag_clean()
{
    robot_program_2p4g_data[4] = 0;
}

uint16_t robot_program_get_robot_posi()
{
    int ret,posi;
    posi = robot_posi_data_once(0,3);
    DBG_PRINT("posi =",posi);
    return ret;
}

uint16_t robot_program_get_robot_touch()
{
    uint16_t touch_status[3];
    int i;
    int ret = 0;
    robot_key_get_status(touch_status);
    for(i=0;i<3;i++)
    {
        if(touch_status[i]==KEY_DOWN)
        {
            ret = i + 1;
            break;
        }
    }
    return ret;
}


uint16_t robot_program_get_touch()
{
    int i;
    int ret = 0;
    //ret = get_iic_keyidx();
    DBG_PRINT("touch ret =",ret);
    return ret;
}

uint16_t robot_program_get_robot_status(uint8_t data[])
{
    data[0] = robot_program_get_robot_posi();
    data[1] = robot_program_get_touch();
    data[2] = robot_program_get_robot_irdata(NULL,NULL);
    data[3] = robot_program_get_robot_2p4gdata(NULL);
    data[4] = os_get_net_connect();
    //robot_program_key_clr();
    return 4;
}

uint16_t robot_program_emojistatus(uint8_t data[])
{

}

uint16_t robot_program_get_asr_status(uint8_t data[])
{

}

void robot_program_back(uint8_t backtype,uint8_t times_flag)
{
    uint8_t backdata[24];
    uint16_t backsz;
    backdata[0] = 0xF4;
    backdata[1] = 0x4F;
    backdata[2] = 0x04;
    backdata[3] = backtype;
    backdata[4] =times_flag;
    switch(backtype)
    {
    case 5://普通
        backsz = robot_program_get_robot_status(&backdata[5]);
        break;
    case 6:
        break;
    case 7://无线
        backsz = robot_program_get_2p4gdata(&backdata[5]);
        robot_program_2p4gflag_clean();
        break;
    case 8://播放状态
        backsz = robot_program_get_voice_status(&backdata[5]);
        break;
    case 9://麦克风
        backsz = robot_program_get_asr_status(&backdata[5]);
        break;
    case 10://传感器
        backsz = robot_program_get_external_status(&backdata[5]);
        break;
    }
    if(backsz)
    {
        backsz += 6;
        APP_SEND(backdata,backsz);
    }
}


void robot_program_run_start()
{
    //清除2.4G数据
    robot_program_2p4gdata_clean();

    //表情模式 速度中，赚眼球表情
    robot_program_emoji_set_mode(ROBOT_PROGRAM_EMOJI_MODE);

    //打断当前声音，打断模式为打断，音量中（3）
    robot_program_voice_breakmode_set(ROBOT_PROGRAM_VOICD_STOP_BREAK);
    robot_program_voice_break_make(ROBOT_PROGRAM_VOICD_STOP_BREAK);
    robot_program_voice_volume_set(3);

    //动作，站立
    robot_action_program_play(2,0,0);
}

//运行开始和结束的工作几乎是一样的
//运行结束要多一边如果倒下就爬起来
void robot_program_run_stop()
{
    robot_program_run_start();
}

void robot_program_command_make(int32_t com,uint8_t para[])
{
    int back = 5;//默认
    switch(com)
    {
    case 1://系统
        back = robot_program_sys_command_make(para[0]);
        break;
    case 2://动作
        robot_program_action_make(para[0],para[1],para[2]);
        break;
    case 3://声音
        robot_program_voice_make(para[0],para[1],&para[2]);
        break;
    case 4://传感器
        robot_program_external_make(para[0],para[1],&para[2]);
        back = 10;
        break;
    case 5://语音
        robot_program_asr_make(para[0],para[1],&para[2]);
        break;
    case 6://网络
        back = robot_program_network(&para[0]);
        break;
    case 9://特殊情况
        back = 8;
        break;
    case 0x0A://群演
        robot_groupshow_action_make(para[0],para[1],para[2]);
        break;
    }
    if(back)
    {
        DBG_PRINT("program back : ",back);
        robot_program_back(back,times_flag);
    }
    if (times_flag == 0xFF) {
        times_flag = 0;
    } else {
        times_flag++;
    }
}
