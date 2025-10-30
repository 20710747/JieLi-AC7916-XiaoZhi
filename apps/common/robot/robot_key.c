#include "robot_main.h"
#include "robot_debug.h"
#include "robot_key.h"
#include "robot_bus.h"
#include "robot_adapter.h"

#if 1
#define robot_key_size 0
const int robot_key_io[robot_key_size]=
{
};
#elif 1

#define robot_key_size 8
const int robot_key_io[robot_key_size]=
{
    /*
    IO_PORTC_00,IO_PORTC_04,IO_PORTC_02,IO_PORTC_06,
    IO_PORTA_03,IO_PORTA_00,IO_PORTA_05,IO_PORTA_08,
    */
    IO_PORTC_03,IO_PORTC_04,IO_PORTC_05,IO_PORTC_06,
    IO_PORTA_03,IO_PORTA_04,IO_PORTA_05,IO_PORTA_06,
};
#else
#define robot_key_size 2
const int robot_key_io[robot_key_size]=
{
    IO_PORTC_00,
    IO_PORTA_06,
};
#endif

int16_t robot_key_downflag[robot_key_size];
int16_t robot_key_status[robot_key_size];
int16_t robot_key_touch[robot_key_size];
int16_t robot_key_relax[robot_key_size];
int16_t robot_key_last[robot_key_size];
int robot_key_switch = 0;
int robot_key_lock = 0;
int debug_show_once = 0;
int debug_show = 0;

int robot_key_update_det = 0;
int robot_key_update_key = 0;
int robot_key_update_status = 0;
/*
按下返回1
抬起返回0
*/
int robot_key_detect(int keyid)
{
    int ret = (!gpio_read(robot_key_io[keyid]));
    if((robot_key_last[keyid]!=ret)||(debug_show))
    {
        if(robot_key_switch==2)
            DBG2_PRINT("key = ",keyid,ret);
        robot_key_last[keyid] = ret;
    }

    return ret;
}

#define ROBOT_KEY_TOUCH_TIME 5
#define ROBOT_KEY_RELAX_CLR_TIME 5
#define ROBOT_KEY_RELAX_DOWNFLAG_CLR_TIME 200//4s
#define ROBOT_KEY_LONG_TIME 100

void robot_key_update(int key,int status)
{
    robot_key_update_key = key;
    robot_key_update_status = status;
    robot_key_update_det  = 1;
    //DBG2_PRINT("robot_key_update",key,status);
}



int debug_show_set()
{
    debug_show_once = 1;
    DBG_PRINT("debug_show_set ",1);
}
u32 robot_key_gpio_debugone(u32 a,u32 b,char tag[])
{
    if((a!=b)||(debug_show))
        DBGU32_PRINT(tag,a);
    return a;
}

void robot_key_gpio_debug()
{
    static u32 debug_data[8]={0,0,0,0,0,0,0,0};
    debug_data[0] = robot_key_gpio_debugone(JL_PORTA->DIR,debug_data[0],"A-DIR");
    debug_data[1] = robot_key_gpio_debugone(JL_PORTA->DIE,debug_data[1],"A-DIE");
    debug_data[2] = robot_key_gpio_debugone(JL_PORTA->PU,debug_data[2],"A-PU");
    debug_data[3] = robot_key_gpio_debugone(JL_PORTA->PD,debug_data[3],"A-PD");
    debug_data[4] = robot_key_gpio_debugone(JL_PORTC->DIR,debug_data[4],"C-DIR");
    debug_data[5] = robot_key_gpio_debugone(JL_PORTC->DIE,debug_data[5],"C-DIE");
    debug_data[6] = robot_key_gpio_debugone(JL_PORTC->PU,debug_data[6],"C-PU");
    debug_data[7] = robot_key_gpio_debugone(JL_PORTC->PD,debug_data[7],"C-PD");
}

void robot_key_timer_once()//20ms
{
    int i;
    int key_det;
    robot_key_lock = 1;
    debug_show = debug_show_once;
    debug_show_once = 0;
    robot_key_gpio_debug();

    for(i=0;i<robot_key_size;i++)
    {
        key_det = robot_key_detect(i);
        switch(robot_key_status[i])
        {
        case KEY_IDLE:
            if(key_det)
            {
                robot_key_touch[i]++;
                robot_key_relax[i] = 0;
                if(robot_key_touch[i]>=ROBOT_KEY_TOUCH_TIME)
                {

                    robot_key_status[i] = KEY_HOLD;
                    robot_key_downflag[i] = KEY_DOWN;
                    robot_key_touch[i] = 0;
                    robot_key_relax[i] = 0;
                    robot_key_update(i,1);
                    if(robot_key_switch)
                        DBG_PRINT("KEY DOWN",i);
                }
            }
            else
            {
                robot_key_relax[i]++;
                if(robot_key_relax[i]>=ROBOT_KEY_RELAX_CLR_TIME)
                    robot_key_touch[i] = 0;
                if(robot_key_relax[i]>=ROBOT_KEY_RELAX_DOWNFLAG_CLR_TIME)
                    robot_key_downflag[i] = KEY_IDLE;
            }
            break;
        case KEY_HOLD:
            if(!key_det)
            {
                robot_key_relax[i]++;
                if(robot_key_relax[i]>=ROBOT_KEY_RELAX_CLR_TIME)
                {
                    robot_key_status[i] = KEY_IDLE;
                    robot_key_touch[i] = 0;
                    robot_key_relax[i] = 0;
                    robot_key_update(i,2);
                    if(robot_key_switch)
                        DBG_PRINT("KEY UP",i);
                }
            }
            else
            {
                robot_key_touch[i]++;
                if(robot_key_relax[i])
                    robot_key_relax[i]--;
                if(robot_key_touch[i]>=ROBOT_KEY_LONG_TIME)
                {
                    robot_key_status[i] = KEY_LONG;
                    robot_key_touch[i] = 0;
                    robot_key_relax[i] = 0;
                }
            }
            break;
        case KEY_LONG:
            if(!key_det)
            {
                robot_key_relax[i]++;
                if(robot_key_relax[i]>=ROBOT_KEY_RELAX_CLR_TIME)
                {
                    robot_key_update(i,2);
                    if(robot_key_switch)
                        DBG_PRINT("KEY UP",i);
                    robot_key_status[i] = KEY_IDLE;
                    robot_key_touch[i] = 0;
                    robot_key_relax[i] = 0;
                }
            }
            else
            {
                if(robot_key_relax[i])
                    robot_key_relax[i]--;
                robot_key_touch[i]++;
                robot_key_update(i,3);
                if(robot_key_switch)
                    if(robot_key_touch[i]<10)
                        DBG_PRINT("KEY LONG",i);
            }
        }
    }
    debug_show = 0;
    robot_key_lock = 0;
}

#define KEY_BAN_TIME_DEFAULT 1000

u32 key_ban_time = 0;

void robot_key_ban_set(u32 det)
{
    if(det==0)
        det = KEY_BAN_TIME_DEFAULT;
    if(key_ban_time)
    {
        u32 nowtime = ROBOT_GETTIME_MS();
        if(nowtime+det>key_ban_time)
        {
            key_ban_time = 0;
        }
    }
    if(!key_ban_time)
    {
        DBG_PRINT("KEY BAN SET",det);
        key_ban_time = ROBOT_GETTIME_MS()+det;
    }
}

int robot_key_ban_get()
{
    //暂时不用这个接口。
    //return 0;
    u32 nowtime;
    if(!key_ban_time)
        return 0;
    nowtime = ROBOT_GETTIME_MS();
    if(nowtime > key_ban_time)
    {
        DBG_PRINT("KEY BAN DONE",0);
        key_ban_time = 0;
        return 0;
    }
    //DBGU32_PRINT("now = ",nowtime);
    //DBGU32_PRINT("ban = ",key_ban_time);
    return 1;
}

extern

int robot_key_tone_flag = 0;

extern int robot_ttp_touch_flag;
extern int robot_ttp_touch_key;
extern int robot_ttp_touch_type;

int tbz_touch_use = 1;
int tbz_touch_use_set(int use)
{
    tbz_touch_use = use;
}

#define PROBABILITY_THRESHOLD 333 // 对应1/3概率 (1000 * 1/3 ≈ 333，这里用300便于调试)

// 获取触摸音频文件名
void play_touch_sound(int touch_idx) {
    // 生成随机数决定使用哪种音频 (0-999)
    int last_num = -1;
    char audio[64];
    int rand_val = rand() % 1000;
    int audio_num = rand() % 5 + 1; // 生成1-5的随机数

    if(last_num == audio_num)
    {
        audio_num = audio_num + 1 + rand()%4;
        if(audio_num>5)
            audio_num -= 5;
    }
    last_num = audio_num;
    if (rand_val < PROBABILITY_THRESHOLD) {  // 使用公用音频
        sprintf(audio, "touch0_%d.mp3", audio_num);
    } else {  // 使用触摸点特有音频
        sprintf(audio, "touch%d_%d.mp3", touch_idx, audio_num);
    }
    os_set_play_audio(audio,NULL);
}

void robot_key_data_make()
{
    //DBG_PRINT("robot_key_data_make",robot_key_update_det);
    static u32 key_lasttime = 0;
    static int play_idx = 1;
    extern int speak_face_flag;

    /*
    if(robot_key_ban_get())
    {
        if(robot_key_update_det)
            DBG_PRINT("key ban >>",robot_key_update_key);
        robot_key_update_det = 0;
    }
    */
    //DBG_PRINT("robot_ttp_touch_flag = ",robot_ttp_touch_flag);
    #if 1
    if(robot_ttp_touch_flag)
    {
        robot_key_update_key = robot_ttp_touch_key;
        if(robot_key_update_key)
            robot_key_update_det = 1;
        DBG2_PRINT("touch key = ",robot_ttp_touch_key,robot_key_update_key);
        robot_ttp_touch_flag = 0;

    }
    #endif
    if(!tbz_touch_use)
    {
        robot_key_update_det = 0;
        robot_key_update_key = 0;
        return ;
    }
    if(robot_key_update_det)
    {
        DBG2_PRINT("key make",robot_key_update_key,robot_key_update_status);
        switch(robot_key_update_key)
        {
        case 2://左脸
            //if(speak_face_flag)
            //    robot_action_wink_play(0x43,300,400);
            robot_key_tone_flag = 1;
            play_touch_sound(2);
            //if(play_idx==1)
            //    play_idx++;
            //robot_control_audio_play_send(play_idx+10);
            break;
        case 3://右脸
            //if(speak_face_flag)
            //    robot_action_wink_play(0x53,300,400);
            robot_key_tone_flag = 2;
            play_touch_sound(3);
            //robot_control_audio_play_send(play_idx+10);
            break;
        case 4://下巴
            robot_key_tone_flag = 3;
            play_touch_sound(4);
            //if(play_idx==1)
            //    play_idx++;
            //robot_control_audio_play_send(play_idx+10);
            break;
        case 6://额头
            robot_key_tone_flag = 4;
            play_touch_sound(6);
            //if(play_idx==1)
            //    play_idx++;
            //robot_control_audio_play_send(play_idx+10);
            break;
        }
        play_idx++;
        if(play_idx>=7)
            play_idx = 1;
        robot_key_update_det = 0;
        robot_key_ban_set(0);
    }
}

uint16_t robot_key_get_keysize()
{
    return robot_key_size;
}

uint16_t robot_key_get_status(uint16_t status[])
{
    int i;
    if(robot_key_lock)
    {
        //要用锁吗？
    }
    DBGS_PRINT("key_raw_status : ",robot_key_status,robot_key_size);
    DBGS_PRINT("key_downflag : ",robot_key_downflag,robot_key_size);
    for(i=0;i<robot_key_size;i++)
    {
        if(robot_key_status[i]==KEY_LONG)
            status[i] = KEY_LONG;
        else if(robot_key_downflag[i]==KEY_DOWN)
            status[i] = KEY_DOWN;
        else
            status[i] = KEY_IDLE;
        robot_key_downflag[i] = KEY_IDLE;
    }
    DBGS_PRINT("key_out_status : ",status,robot_key_size);
    return robot_key_size;
}

void robot_key_init()
{
    int i;
    for(i=0;i<robot_key_size;i++)
    {
        gpio_set_direction(robot_key_io[i], 1);//±Ç×Ó
        gpio_set_pull_up(robot_key_io[i], 0);
        gpio_set_pull_down(robot_key_io[i], 1);
        gpio_set_die(robot_key_io[i], 1);
        //gpio_set_hd1(robot_key_io[i], 1);
    }
    memset(robot_key_status,0,sizeof(robot_key_status));
    memset(robot_key_touch,0,sizeof(robot_key_touch));
    memset(robot_key_relax,0,sizeof(robot_key_relax));
    memset(robot_key_downflag,0,sizeof(robot_key_downflag));
    memset(robot_key_last,-1,sizeof(robot_key_last));
    //robot_key_ban_set(15000);
}

void robot_key_test_once()
{
}

void robot_key_switch_set(uint16_t set)
{
    robot_key_switch = set;
}

void robot_key_data_show()
{
    int i,j;
    for(i=0;i<robot_key_size;i++)
    {
        j = robot_key_detect(i);
        if(j)
            DBG2_PRINT("KEY ID DET: ",i,j);
    }
}



int robot_key_buskey_read()
{
    uint8_t para[128];
    uint8_t comm[128];
    uint16_t csz;
    int ret;
    //STR_PRINT("READ R12KEY:\r\n");
    //DBG_PRINT("KEY READ.",0);
    if(robot_key_update_det)
        return 0;
    para[0] = 0x01;
    csz = robot_bus_command_make(0x7F, 0x02, 0x01, 1, para, comm);
    BUS_FLUSH();
    robot_bus_read_flag_set(BUS_RF_ROBOT_KEY,1,25);
    BUS_WRITE(comm,csz);
    if(1)
    {
        gpio_direction_output(IO_PORTD_02, 1);    //    1    0      RXD
        gpio_direction_output(IO_PORTD_03, 0);    //    0    1      TXD
    }
    ROBOT_DELAY_MS(2);
    while(BUS_READING)
    {
        robot_bus_data_read_once();
        ROBOT_DELAY_MS(2);
    }
    ret = bus_readdata;

    if((ret)&&(ret<32))
    {
        if(ret&4)
        {
            robot_key_update_key = 1;
        }
        else if(ret&8)
        {
            robot_key_update_key = 2;
        }
        else if(ret&16)
        {
            robot_key_update_key = 3;
        }
        else if(ret&2)
        {
            robot_key_update_key = 4;
        }
        robot_key_update_det = 1;
        DBG2_PRINT("key = ",ret,robot_key_update_key);
    }
    if(1)
    {
        gpio_direction_output(IO_PORTD_02, 0);    //    1    0      RXD
        gpio_direction_output(IO_PORTD_03, 1);    //    0    1      TXD
    }
   // DBG_PRINT("KEY READ.",1);
    return ret;
}
