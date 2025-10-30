#include "robot_main.h"
#include "robot_debug.h"
#include "robot_adapter.h"
#include "robot_2p4g.h"
#include "robot_groupshow.h"
#include "robot_action.h"
#include "robot_uart.h"

#define UART2P4G_RECV_BUFF_SZ 64
uint8_t uart2p4g_recv_buff[UART2P4G_RECV_BUFF_SZ+2];
int robot_2p4g_switch = 0;
int uart_2p4g_iomode = 0;
int uart_2p4g_iomode_update = 1;
int program_2p4g_ch = 0;

//SetPin闪一下，就当是心跳包用了

void uart2p4g_heartbeat()
{
    static u32 last_timer = 0;
    u32 cur = ROBOT_GETTIME_MS();
    if(cur-last_timer>=5000)
    {
        last_timer = cur;
        RF2P4G_WRITE("hello",5);
    }
}


#if 0
void robot_2p4g_once()
{
    int rsz;
    int end = 1;
    int remote_flag = 0;
    while(1)
    {
        rsz = RF2P4G_READ(uart2p4g_recv_buff,UART2P4G_RECV_BUFF_SZ);
        if(rsz<=0)
            break;
        if((rsz == 18)&&(uart2p4g_recv_buff[0]==0xaa)&&(uart2p4g_recv_buff[1]==0x5b))//配置2.4G模组的回吗，不需要处理，忽略
            break;
        if(robot_2p4g_switch)
            HEXS_PRINT("2p4g_recv",uart2p4g_recv_buff,rsz);
        if(robot_main_rf2p4g_testflag())
            break;
        if(uart2p4g_recv_buff[0]>=0x80)
        {
            DBG_PRINT("uart2p4g_recv_buff[0]",uart2p4g_recv_buff[0]);
            robot_2p4g_groupshow_make(uart2p4g_recv_buff[0]);
        }
        else if(uart2p4g_recv_buff[0]>0x70)
        {
            //robot_2p4g_system
            robot_2p4g_system_make(uart2p4g_recv_buff[0]);
        }
        else if(uart2p4g_recv_buff[0]>=30)
        {
            end = robot_2p4g_remote_make(uart2p4g_recv_buff[0]);
            if(!end)
                remote_flag = 1;
        }
        else
        {
            robot_2p4g_data_make(uart2p4g_recv_buff,rsz);
        }
        if(end)
            break;
    }
    if(remote_flag)
        robot_2p4g_remote_stop();
}
#endif // 0

void robot_2p4g_TestData_Send(uint16_t type)
{
    switch(type)
    {
        case 1:RF2P4G_SEND(0x7c);break;
        case 2:RF2P4G_SEND(0x7d);break;
    }
}

void robot_2p4g_program_data_send(uint8_t idx,uint8_t data)
{
    RF2P4G_SEND(130+idx*25+data);
}

void robot_2p4g_program_data_set(uint8_t idx,uint8_t data,uint8_t program_data[])
{
    if(idx <= 5)
        program_data[idx] = data;
}

void program_rg2p4g_chset(uint16_t ch)
{
    if(ch!=-1)
        program_2p4g_ch = ch;
    RF2P4G_SET_CHANNEL(ch,NULL);
}

void robot_2p4g_switch_set(uint16_t sw)
{
    robot_2p4g_switch = sw;
    if(sw)
        RF2P4G_SET_CHANNEL(0,NULL);
}

void robot_2p4g_system_make(uint8_t cmd)
{

}

int robot_2p4g_remote_make(uint8_t cmd)
{
    int end = 0;
    if(robot_2p4g_switch==0)
        return 0;
    if((cmd >= 0x30)&&(cmd < 0x50))
    {
        robot_irfollow_switch_set(0);
    }

    switch(cmd)
    {
        case 0x30 : robot_action_cmd_play(1,1); break;
        case 0x31 : robot_action_cmd_play(2,1); break;
        case 0x32 : robot_action_cmd_play(4,1); break;
        case 0x33 : robot_action_cmd_play(3,1); break;
        case 0x34 : robot_action_stunt_random_play();end = 1;break;
        case 0x35 : robot_irfollow_switch_set(4);robot_audio_voice_play(4);end = 1;break;
        case 0x55 : robot_irfollow_adjust_ok();break;
    }
    return end;
}

void robot_2p4g_remote_stop()
{
    robot_action_cmd_play(0,0);
}

void robot_2p4g_data_make(uint8_t *data,int sz)
{

}
