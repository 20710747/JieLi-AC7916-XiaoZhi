#include "robot_main.h"
#include "robot_debug.h"
#include "robot_iictouch.h"
#include "app_config.h"
#include "robot_adapter.h"
#include "robot_config.h"
#include "robot_action.h"

#define TOUCH_INIT_IO   IO_PORTC_00
#define TOUCH_SCK       IO_PORTC_01
#define TOUCH_SDA       IO_PORTC_02

//#define pTTP_INT_IN PD4//PAin(7)
#define pTTP_INIT_INPUT   gpio_direction_input(TOUCH_INIT_IO)

#define pTTP_INIT_READ    gpio_read(TOUCH_INIT_IO)

#define pTTP_SCK_DIS_UP     gpio_set_pull_up(TOUCH_SCK,0)
#define pTTP_SCK_DIS_DOWN   gpio_set_pull_down(TOUCH_SCK,0)

//#define pTTP_SCK PD2
#define pTTP_SCK_HIGH gpio_direction_output(TOUCH_SCK,1)
#define pTTP_SCK_LOW  gpio_direction_output(TOUCH_SCK,0)
//为了保证SCK已经拉高后才执行后续操作，在SCK拉高并确认后才执行后续操作。
#define pTTP_SCK_HIGH_TURE {\
                                gpio_direction_output(TOUCH_SCK,1);\
                                pTTP_SCK_INPUT;\
                                while(pTTP_SCK_READ != 1);\
                                pTTP_SCK_OUT;\
                            }
//#define pTTP_SCK_IN_SET   GPIO_SetMode(PD, BIT2, GPIO_MODE_INPUT);
#define pTTP_SCK_INPUT    gpio_direction_input(TOUCH_SCK)
//#define pTTP_SCK_OUT_SET  GPIO_SetMode(PD, BIT2, GPIO_MODE_OUTPUT);
//存疑
#define pTTP_SCK_OUT      gpio_set_direction(TOUCH_SCK,0)
//#define pTTP_SCK_OUT      gpio_direction_output(TOUCH_SCK,1)

//#define pTTP_SDA_IN_SET   GPIO_SetMode(PD, BIT3, GPIO_MODE_INPUT);
#define pTTP_SDA_INPUT    gpio_direction_input(TOUCH_SDA)
//#define pTTP_SDA_OUT_SET  GPIO_SetMode(PD, BIT3, GPIO_MODE_OUTPUT);
#define pTTP_SDA_OUT      gpio_set_direction(TOUCH_SDA,0)
//#define pTTP_SDA_OUT      gpio_direction_output(TOUCH_SDA,1)

#define pTTP_SDA_DIS_UP     gpio_set_pull_up(TOUCH_SDA,0);
#define pTTP_SDA_DIS_DOWN   gpio_set_pull_down(TOUCH_SDA,0);

//#define pTTP_SDA PD3
#define pTTP_SDA_HIGH gpio_direction_output(TOUCH_SDA,1)
#define pTTP_SDA_LOW  gpio_direction_output(TOUCH_SDA,0)

//#define pTTP_SCK_IN PD2
#define pTTP_SCK_READ gpio_read(TOUCH_SCK)
//#define pTTP_SDA_IN PD3
#define pTTP_SDA_READ gpio_read(TOUCH_SDA)


#define W_ADDR1 0XA6
#define R_ADDR1 0XA7

#define W_ADDR2 0XA0
#define R_ADDR2 0XA1

#define W_ADDR3 0XA2
#define R_ADDR3 0XA3

#define W_ADDR4 0XA4
#define R_ADDR4 0XA5


#define ROBOT_TTP_IIC_DELAY(x) delay_us(x)


const int W_ADDR[3] = {W_ADDR1,W_ADDR3,W_ADDR4};
const int R_ADDR[3] = {R_ADDR1,R_ADDR3,R_ADDR4};
/*
下巴 1 2
右脸 1 1
左脸 3 1 旁边有舵机
额头 2 2
*/
const int threshold[3][2] = {{160,240},{160,160},{180,160}};
const int TOUCH_SIZE = 3;

int ttp_iic_slave = 0;
int ttp_iic_debug = 1;
unsigned char gBuF[4]={0};
extern int touchprogram;
uint16_t robot_touch_size;
int iictouch_flag = 0;

int robot_ttp_touch_flag = 0;
int robot_ttp_touch_key = 0;
int robot_ttp_touch_type = 0;
static u32 presstime = 0;

void robot_ttp_i2c_delay(void)
{
      //ROBOT_TTP_IIC_DELAY(1);
        ROBOT_DELAY_US(1);
}

void robot_setBuF(unsigned char dat1,unsigned char dat2,unsigned char dat3)
{
        gBuF[0] = dat1;
        gBuF[1] = dat2;
        gBuF[2] = dat3;
}
void robot_ttp_i2c_start(void)
{
        pTTP_SDA_HIGH;//SDA输出模式，输出高电平
        pTTP_SCK_HIGH_TURE;
        robot_ttp_i2c_delay();

        pTTP_SDA_LOW;//SDA拉低电平
        robot_ttp_i2c_delay();
        pTTP_SCK_LOW;//SCK拉低电平
}
//------------------------------------------------------
void robot_ttp_i2c_stop(void)
{
        pTTP_SCK_LOW;//SCK拉低电平
        pTTP_SDA_LOW;//SDA拉低电平
        robot_ttp_i2c_delay();
        pTTP_SCK_HIGH_TURE;

        robot_ttp_i2c_delay();
        pTTP_SDA_HIGH;//SDA拉高电平
}

//==================================
//等待应答信号到来
//用于发送模式下，发送8位后，等待器件应答第9位
//返回值：1，接收应答失败
//        0，接收应答成功
//==================================
unsigned char robot_ttp_i2c_readack(void)
{
        u8 ret;
        pTTP_SCK_LOW;//SCK输出模式，拉低电平
        pTTP_SDA_HIGH;//SDA输出模式，拉高电平
        robot_ttp_i2c_delay();

        pTTP_SCK_HIGH;//SCK输出模式，拉高电平
        pTTP_SCK_INPUT;//SCK输入模式
        pTTP_SDA_INPUT;//SDA输入模式
        while(pTTP_SCK_READ != 1);//等到SCK拉到高电平
        pTTP_SCK_OUT;//SCK输出模式，拉高电平
        robot_ttp_i2c_delay();

        if(pTTP_SDA_READ)
            ret = 1;
        else
            ret = 0;
        //pTTP_SDA_OUT;//SDA输出模式，拉高电平
        //if(ret)
            //DBG_PRINT("read ack = ",ret);
        return ret;
}

//==================================
//发送ACK应答
//用于读取模式(SDA为in)读了8位器件数据后，在第9位给出一个应答，继续读
//==================================
void robot_ttp_i2c_sendack(void)
{
        pTTP_SCK_LOW;//SCK输出模式，拉低电平
        pTTP_SDA_LOW;//SDA输出模式，拉低电平表示应答
        robot_ttp_i2c_delay();
        pTTP_SCK_HIGH;//SCK输出模式，拉高电平
        //robot_ttp_i2c_delay();
        //pTTP_SCK_LOW;//SCK拉低，应答结束
        //STR_PRINT("444444444444444\r\n");
}

//==================================
//不产生ACK应答
//用于读取模式(SDA为in)读了8位器件数据后，在第9位给出一个应答，不再继续读
//==================================
void robot_ttp_i2c_sendnoack(void)
{
        pTTP_SCK_LOW;//SCK输出模式，拉低电平
        pTTP_SDA_HIGH;//SDA输出模式，拉高电平表示不应答
        robot_ttp_i2c_delay();
        pTTP_SCK_HIGH;//SCK输出模式，拉高电平
        //robot_ttp_i2c_delay();
        //pTTP_SCK_LOW;//SCK拉低，不应答信号发送结束
        //STR_PRINT("55555555555555555\r\n");
}


void robot_ttp_i2c_sendbyte(unsigned char sdata)
{
        unsigned char i;
        for(i=0; i<8; i++)
        {
            pTTP_SCK_LOW;//SCK输出模式，拉低电平

            if(sdata & 0x80)
                pTTP_SDA_HIGH;//SDA拉高，表示1
            else
                pTTP_SDA_LOW;//SDA拉低，表示0
            robot_ttp_i2c_delay();

            pTTP_SCK_HIGH_TURE;
            robot_ttp_i2c_delay();
            sdata <<= 1;
        }
}
//------------------------------------------------------

unsigned char robot_ttp_i2c_readbyte(void)
{
        unsigned char i,sdata;
        sdata = 0;
        for(i=0; i<8; i++)
        {
            pTTP_SCK_LOW;//SCK输出模式，拉低电平
            pTTP_SDA_HIGH;//SDA输出模式，拉高电平     ??
            robot_ttp_i2c_delay();
            sdata <<= 1;

            pTTP_SCK_HIGH;//SCK输出模式，拉高电平
            pTTP_SDA_INPUT;//SDA输入模式

            pTTP_SCK_INPUT;//SCK输入模式
            while(pTTP_SCK_READ != 1);//等待SCK拉高
            pTTP_SCK_OUT;//SCK输出模式
            robot_ttp_i2c_delay();

            if(pTTP_SDA_READ)
                sdata |= 0x01;
        }
        pTTP_SDA_OUT;//SDA输出模式，拉高电平
        return sdata;
}

unsigned char robot_ttp_writedata(unsigned char addr, unsigned char *buf, unsigned char length)
{
            unsigned char i;
            //DBG_PRINTF("robot_ttp_writedata addr = %d buf[] = %d %d %d",addr,buf[0],buf[1],buf[2]);
            robot_ttp_i2c_start();
            robot_ttp_i2c_sendbyte(addr);

            if(robot_ttp_i2c_readack())
            {
                    robot_ttp_i2c_stop();
                    return 1;
            }

            for(i=0; i<length; i++)
            {
                robot_ttp_i2c_sendbyte(buf[i]);
                if(robot_ttp_i2c_readack())
                {
                        robot_ttp_i2c_stop();
                        return 1;
                }
            }

            robot_ttp_i2c_stop();
            return 0;
}

unsigned char robot_ttp_readdata(unsigned char addr, unsigned char *buf, unsigned char length)
{
    unsigned char i;
    //DBG_PRINTF("robot_ttp_readdata [addr = %d]",addr);
    robot_ttp_i2c_start();
    robot_ttp_i2c_sendbyte(addr);
    if(robot_ttp_i2c_readack())
    {
        //DBG_PRINTF("[addr = %d] Read Fail",addr);
        robot_ttp_i2c_stop();
        return 1;
    }
    for(i=0; i<length; i++)
    {
        buf[i] = robot_ttp_i2c_readbyte();
        if(i < length - 1)
            robot_ttp_i2c_sendack();
        else
            robot_ttp_i2c_sendnoack();
    }
    //DBG_PRINTF("[addr = %d] Read OK! length = %d.",addr,length);



            #if 0
            extern u32 robot_action_end_time_bankey;
            if(robot_action_end_time_bankey)
            {
                u32 nowtime =  ROBOT_GETTIME_MS();
                DBG_PRINT_UINT32("ban time = ",robot_action_end_time_bankey);
                DBG_PRINT_UINT32("now time = ",nowtime);
                DBG_PRINT_UINT32("det time = ",nowtime - robot_action_end_time_bankey);
                if(nowtime - robot_action_end_time_bankey<1000)
                {
                    DBG_PRINT("key ban >>",0);
                    buf[0] = 0;
                    buf[1] = 0;
                }
                else
                {
                    robot_action_end_time_bankey = 0;
                }
            }
            #elif 0
            if(robot_action_actionend_bantime_judge())
            {
                DBG_PRINT("touch key ban >>",0);
                buf[0] = 0;
                buf[1] = 0;
            }
            #endif
			robot_ttp_i2c_stop();

            return 0;
}
int program_key_value;

void robot_program_key_make(int key)
{
    if(key)
    {
        program_key_value = key;
    }
}

void robot_program_key_clr()
{
    program_key_value = 0;
}


int current_volume = 3;

void robot_normal_key_make(int key,int type)
{

    switch(robot_ttp_touch_key)
    {
        case 1://book
            if(Robot_vBatLow_Flag_Get()){
                ex_audio_play("lownobook.mp3",NULL);
                return;
            }
            robot_ai_book_start();
        break;
        case 2://vol-
            if(type == 1)
                tbz_dac_change_volume(-1,1,1);
            else
                ex_app_music_play_song(2);
            break;
        case 4://music play/pause
            if(Robot_vBatLow_Flag_Get()){
                ex_audio_play("lownomusic.mp3",NULL);
                return;
            }
            ex_app_music_play_song(0);
            break;
        case 8://vol+
            if(type == 1)
                tbz_dac_change_volume(1,1,1);
            else
                ex_app_music_play_song(1);
            break;
        case 16://asr
            if(Robot_vBatLow_Flag_Get()){
                ex_audio_play("lownoasr.mp3",NULL);
                return;
            }
            robot_aiasr_mode_switch();
            break;
        case 17:
            DBG_PRINT("TIME = ",presstime);
            if (type == 2 && presstime >= 3000) { // Modify to trigger only if long pressed for 3 seconds

                if(iictouch_flag == 0){
                    iictouch_flag = 1;
                    robot_groupshow_groupshow_Set(1);
                }else{
                    iictouch_flag = 0;
                    robot_groupshow_groupshow_Set(0);
                }
                presstime = 0;
            }
            break;

    }
}

void robot_ttp_action_make()
{
    static int asr_switch_state = 0;
    if(ttp_iic_debug)
        return ;
    if(robot_ttp_touch_flag)
    {
        if(robot_program_flag_get()|| robot_app_flag_get())
        {
            robot_program_key_make(robot_ttp_touch_key);
        }
        else
        {
            robot_normal_key_make(robot_ttp_touch_key,robot_ttp_touch_type);
        }
        robot_ttp_touch_flag = 0;
    }
}

void robot_ttp_key_send(int key,int type)
{
    if(robot_key_ban_get())
    {
        DBG_PRINTF("key ban >> key = %d type = %d",key,type);
    }
    else
    {
        robot_ttp_touch_key = key;
        robot_ttp_touch_type = type;
        robot_ttp_touch_flag = (type==0)?(0):(1);
        DBG_PRINTF("key send >> key = %d flag = %d",robot_ttp_touch_key,robot_ttp_touch_flag);
    }

}

int robot_ttp_key_get()
{
    return robot_ttp_touch_flag?(robot_ttp_touch_key):(0);
}

void robot_iictouch_set_sensitivity(int idx,int key,int sen)
{
    int i;
    if(key==0)//唤醒灵敏度
    {
        robot_setBuF(0xe0,sen,0);
        robot_ttp_writedata(W_ADDR[idx],gBuF,3); //Write WakeUp Threshold
    }
    else//按键灵敏度
    {
        for(i=0;i<5;i++)
        {
            if(key&(1<<i))
            {
                if(sen==-1)
                    robot_setBuF(0xc0+i,0,2);
                else
                    robot_setBuF(0xc0+i,sen%0xff,sen/0x100);
                DBG2_PRINT("TTP SEN SET ",i,sen);
                robot_ttp_writedata(W_ADDR[idx],gBuF,3); //Write WakeUp Threshold
            }
        }
    }
}


uint16_t keyrobot_iictouch_get_sensitivity(int keyidx)
{
    return 0;
}

void robot_iictouch_get_sensitivity()
{

}
#if 1
// 宏定义配置参数
#define KEY4_SHIELD_DURATION_MS   100    // 2/3号按键按下后屏蔽4号按键的持续时间
#define KEY4_DELAY_TRIGGER_MS     200    // 4号按键延时触发的时间

// 状态变量定义
static struct {
    u32 shield_4_until;      // 屏蔽4号按键直到的时间戳
    u32 key4_press_time;     // 4号按键按下时的时间戳
    bool key4_pending;       // 4号按键等待触发标志
    bool key4_triggered;     // 4号按键已触发标志
} key_state = {0, 0, false, false};

void robot_ttp_key_det(int last_key, int now_key)
{
    // 定义按键掩码数组，每个元素对应一个按键的掩码
    const unsigned short KEY_MASKS[] = {
        0x0000,   // 留空
        0x0100,   // 左脸2
        0x0001,   // 右脸3
        0x0002,   // 下巴4
        0x0000,   // 留空
        0x0020,   // 额头6
        0x0000,   // 留空
        0x0000,   // 留空
    };

    u32 current_time = os_get_time_ms();

    // 检查4号按键延时触发是否到期
    if (key_state.key4_pending &&
        current_time - key_state.key4_press_time > KEY4_DELAY_TRIGGER_MS) {
        if (!(key_state.shield_4_until > current_time)) {
            // 触发4号按键按下
            DBG_PRINTF("KEY[4] DOWN (CONFIRMED after delay)");
            key_state.key4_triggered = true;
            robot_ttp_key_send(4,1);
        } else {
            //DBG_PRINTF("KEY[4] CANCELLED (shielded during delay)");
        }
        key_state.key4_pending = false;
    }

    // 计算变化位（只关注有效位）
    unsigned short key_changed = (now_key ^ last_key) & 0x3333;

    // 循环处理每个按键
    for(int i = 0; i < 8; i++) {
        unsigned short mask = KEY_MASKS[i];
        int key_id = i + 1;

        // 检查这个按键是否发生了变化
        if(key_changed & mask) {
            // 检查是按下还是松开
            if(now_key & mask) {
                // 按键按下处理
                if(key_id == 2 || key_id == 3) {
                    // 2或3号按键按下，屏蔽4号按键一段时间
                    key_state.shield_4_until = current_time + KEY4_SHIELD_DURATION_MS;
                    // 取消 pending 的4号按键触发
                    if (key_state.key4_pending) {
                        //DBG_PRINTF("KEY[4] CANCELLED (due to KEY[%d] press)", key_id);
                        key_state.key4_pending = false;
                    }
                    robot_ttp_key_send(key_id,1);
                    DBG_PRINTF("KEY[%d] DOWN", key_id);
                    //DBG_PRINTF("shielding KEY[4] for %dms)", KEY4_SHIELD_DURATION_MS);
                }
                else if(key_id == 4) {
                    if(key_state.shield_4_until > current_time) {
                        // 在屏蔽期内，忽略此次4号按键按下
                        //DBG_PRINTF("KEY[4] IGNORED (shielded by KEY[2/3])");
                        continue;
                    } else {
                        // 记录按下时间，延迟处理
                        DBG_PRINTF("KEY[%d] DOWN MAYBE", key_id);
                        key_state.key4_press_time = current_time;
                        key_state.key4_pending = true;
                        //DBG_PRINTF("KEY[4] POSSIBLE DOWN (waiting %dms confirmation)", KEY4_DELAY_TRIGGER_MS);
                    }
                }
                else {
                    // 其他按键正常处理
                    robot_ttp_key_send(key_id,1);
                    DBG_PRINTF("KEY[%d] DOWN", key_id);
                }
            } else {
                // 按键松开处理
                if((key_state.key4_pending)&&(key_id==4))
                {
                    DBG_PRINTF("KEY[%d] UP time = %d", key_id,current_time-key_state.key4_press_time);
                }
                #if 0
                if(key_id == 4) {
                    if(key_state.key4_triggered) {
                        DBG_PRINTF("KEY[4] UP");
                        key_state.key4_triggered = false;
                    } else if (key_state.key4_pending) {
                        DBG_PRINTF("KEY[4] CANCELLED (released before confirmation)");
                        key_state.key4_pending = false;
                    }
                }
                else if(key_id == 2 || key_id == 3) {
                    // 2或3号按键松开，正常处理
                    DBG_PRINTF("KEY[%d] UP", key_id);
                }
                else {
                    // 其他按键正常处理
                    DBG_PRINTF("KEY[%d] UP", key_id);
                }
                #endif
            }
        }
    }
}
#else

void robot_ttp_key_det(int last_key,int now_key)
{
     // 定义按键掩码数组，每个元素对应一个按键的掩码
    const unsigned short KEY_MASKS[] = {
        0x0001,   // 按键4对应位1
        0x0008,   // 按键3对应位2
        0x0010,   // 按键4对应位3
        0x0080,   // 按键3对应位4
        0x0100,   // 按键2对应位5
        0x0800,   // 按键1对应位6
        0x1000,   // 按键2对应位7
        0x8000,   // 按键1对应位8
    };

    // 计算变化位（只关注有效位）
    unsigned short key_changed = (now_key ^ last_key) & 0x9999;

    // 循环处理每个按键
    for(int i = 0; i < 8; i++) {
        unsigned short mask = KEY_MASKS[i];

        // 检查这个按键是否发生了变化
        if(key_changed & mask) {
            // 检查是按下还是松开
            if(now_key & mask) {
                // 按键i+1新按下
                //DBG_PRINTF("KEY[%d] DOWN",i+1);
            } else {
                // 按键i+1新松开
                //DBG_PRINTF("KEY[%d] UP",i+1);
            }
        }
    }
}
#endif
int robot_ttp_debug_flag = 0;

void robot_ttp_debug_set(int flag)
{
    robot_ttp_debug_flag = flag;
}

void robot_ttp_server(void)
{
    //static int onkey = 0;
    static int count = 0;
    int i;
    static int last_key = 0;
    int temp = 0;
    static int reread_flag = 1;//如果读错了，就抛弃数据，然后设置这个flag，下次重读一次。
    int err = 0;
    if(robot_key_ban_get())
        return ;
    reread_flag = 1;
    if(robot_ttp_debug_flag==1)
    {

        robot_ttp_debug_flag = 2;
        reread_flag = 1;
    }
    if(robot_ttp_debug_flag>=2)
    {
        DBG_PRINTF("iic det [%d]",count++);
    }
    else
    {
        count = 0;
    }

    //DBG_PRINT("ttp reread_flag = %d",reread_flag);
    if ((pTTP_INIT_READ == 0)||(reread_flag)) {
        int now_key = 0;
        for(i=0;i<TOUCH_SIZE;i++)
        {
            if(err)
                return ;
            robot_setBuF(0,0,0);
            robot_ttp_readdata(R_ADDR[i], gBuF, 2);
            if(robot_ttp_debug_flag>=2)
            {
                DBG_PRINTF("READ[%d] >> %d %d",i+1,gBuF[0],gBuF[1]);
            }
            if((gBuF[0]!=192)&&(gBuF[0]!=128))
            {
                now_key += (last_key&(0x000F<<(i*4)));
            }
            else
            {
                //gBuF[1] |= ((gBuF[1] & 0x10) >> 1);//tp4 和 tp5合并 因为有些板子接了TP4,有些接了TP5.
                gBuF[1]&=0x0003;
                now_key += (gBuF[1]<<(i*4));
            }
            //if(robot_ttp_debug_flag>=2)
            //{
            //    DBG_PRINTF("now_key = err = %d %d",now_key,err);
            //}
        }
        if(err==0)
        {
            if(robot_ttp_debug_flag>=2)
            {
                DBG_PRINTF("robot_ttp_key_det >> %x %x",last_key,now_key);
            }
            robot_ttp_key_det(last_key,now_key);
            last_key = now_key;
            reread_flag = 0;
        }
        else
        {
            //DBG_PRINTF("err ??[%d]",gBuF[0]);
            reread_flag = 1;
        }
    }
    if(robot_ttp_debug_flag == 2)
        robot_ttp_debug_flag = 0;
}

int get_iic_keyidx()
{
    const touch_value[] = {0,1,2,4,8,16,17};
    int touch_sz = 7;
    int i;
    for (int i = 0; i < touch_sz; i++)
    {
        if (program_key_value == touch_value[i])
        {
            return i;
        }
    }
    return 0;
}

/*
void robot_ttp_i2c_set_sensitivity(u8 order,u8 value,u8 const)
{

}*/
void robot_test_timer_unit()
{
     robot_ttp_server();
}

void robot_ttp_debug_readsen(int tidx,int kidx,char tag[])
{
    robot_setBuF(0x5a, 0xa5, kidx); // 设置命令缓冲区，准备读取 0xe0 参数
    robot_ttp_writedata(W_ADDR[tidx],gBuF,3);
    robot_ttp_readdata(R_ADDR[tidx],gBuF,3);
    HEXS_PRINT(tag,gBuF,3);
    ROBOT_DELAY_MS(50);
}

void robot_ttp_i2c_init()
{
        //int ttp_i2c_debug = 1;
    int i;
        pTTP_INIT_INPUT;
        pTTP_SCK_DIS_UP;//TOUCH_SCK口上拉输入
        pTTP_SCK_DIS_DOWN;//TOUCH_SCK口下拉输入
        pTTP_SDA_DIS_UP;//TOUCH_SDA口上拉输入
        pTTP_SDA_DIS_DOWN;//TOUCH_SDA口下拉输入
        pTTP_SDA_HIGH;//TOUCH_SDA口输出模式，输出高电平
        pTTP_SCK_HIGH;//TOUCH_SCK口输出模式，输出高电平
        robot_ttp_i2c_delay();

/*
上电初始化写入 3 类参数，可设置每个按键的灵敏度、唤醒灵敏度、休眠设置。

按键灵敏度命令：0xC0-0xCB ,第 2，3 个参数,16BIT 灵敏度，低字节在前，高
字节在后。

唤醒灵敏度命令：0xE0,第 2，3 个参数,16BIT 灵敏度，低字节在前，高字节在后。

休眠开启命令： (0x88,0x00,0x00)
休眠关闭命令： (0x80,0x00,0x00)
上电不写入参数：默认按键灵敏度为 60，唤醒灵敏度为 10，休眠开启
*/
//越小越灵敏  最低设置为30 对应的是0x1E

//---------------------按键灵敏度设置




//---------------------按键灵敏度设置
    DBG_PRINTF("IIC Config Deubg = addr = %d %d",W_ADDR[0],W_ADDR[1]);
    for(i=0;i<TOUCH_SIZE;i++)
    {
        robot_setBuF(0x80,0,0);
        robot_ttp_writedata(W_ADDR[i],gBuF,3); //Write WakeUp Threshold
        for(int j=0;j<2;j++)
        {
            robot_setBuF(0xc0+j,threshold[i][j]%256,threshold[i][j]/256);
            robot_ttp_writedata(W_ADDR[i],gBuF,3); //Write TP1 Threshold
            ROBOT_DELAY_MS(10);

            robot_setBuF(0xe0+j,0,0);
            robot_ttp_writedata(W_ADDR[i],gBuF,3); //Write TP1 Threshold
            ROBOT_DELAY_MS(10);
        }

    }
    DBG_PRINTF("IIC Config Deubg Write Done.");

	//............读取灵敏度
	if(ttp_iic_debug)
    {
        for(i=0;i<TOUCH_SIZE;i++)
        {
            DBG_PRINTF("touch[%d] threshold:",i+1);
            robot_ttp_debug_readsen(i,0xc0," data_c0 = ");
            robot_ttp_debug_readsen(i,0xc1," data_c1 = ");
            //robot_ttp_debug_readsen(i,0xc2," data_c2 = ");
            //robot_ttp_debug_readsen(i,0xc3," data_c3 = ");
            //robot_ttp_debug_readsen(i,0xc4," data_c4 = ");

            robot_ttp_debug_readsen(i,0xe0," data_e0 = ");
            robot_ttp_debug_readsen(i,0xe1," data_e1 = ");
            //robot_ttp_debug_readsen(i,0xe2," data_e2 = ");
            //robot_ttp_debug_readsen(i,0xe3," data_e3 = ");
            //robot_ttp_debug_readsen(i,0xe4," data_e4 = ");
        }
	}
	for(i=0;i<TOUCH_SIZE;i++)
    {
        robot_setBuF(0x5a,0x00,0x00);		//读取完之后需要关闭读取命令
        robot_ttp_writedata(W_ADDR[i],gBuF,3);
    }
    DBG_PRINTF("ttp_i2c_init SCCUSE");
}



