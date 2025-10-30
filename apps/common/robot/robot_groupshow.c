#include "robot_main.h"
#include "robot_uart.h"
#include "robot_debug.h"
#include "robot_groupshow.h"
#include "robot_adapter.h"
#include "robot_app.h"
#include "robot_program.h"



int uart2p4g_cflag = 0;
int uart2p4g_sflag = 0;
int uart2p4g_recvtime = 0;
int uart2p4g_showsubflag = 0;


void uart2p4g_flag_set(int flag)
{
    switch(flag)
    {
    case GroudShowIdle:
        if(uart2p4g_sflag) RF2P4G_SEND(0xfb);
        if(uart2p4g_cflag==2) RF2P4G_SEND(0xfc);
        if((uart2p4g_sflag)||(uart2p4g_cflag==2))
            os_set_play_audio("GSExit.mp3", NULL);
        uart2p4g_sflag = 0;
        uart2p4g_cflag = uart2p4g_showsubflag;
        break;
    case GroudShowMain:
        uart2p4g_sflag = 1;
        uart2p4g_cflag = 0;
        os_set_play_audio("GSMainMode.mp3", NULL);
        RF2P4G_SET_CHANNEL(0,NULL);
        RF2P4G_SEND(0xf9);
        break;
    case GroudShowSubWait:
        uart2p4g_showsubflag = 1;
        uart2p4g_sflag = 0;
        uart2p4g_cflag = 1;
        RF2P4G_SET_CHANNEL(0,NULL);
        RF2P4G_SEND(0xfa);
        STR_PRINT("GroudShowSubWait\r\n");
        break;
    case GroudShowSubSet:
        uart2p4g_sflag = 0;
        uart2p4g_cflag = 2;
        os_set_play_audio("GSSlaveMode.mp3", NULL);
        break;
    }
}

void uart2p4g_showsubflag_set(int set)
{
    if(set == uart2p4g_showsubflag)
        return ;
    uart2p4g_showsubflag = set;
    if(set)
    {
        if((uart2p4g_sflag==0)&&(uart2p4g_cflag==0))
            uart2p4g_flag_set(GroudShowSubWait);
    }
    else
    {
        uart2p4g_flag_set(GroudShowIdle);
    }
}

int GroupShow_Onset()
{
    return ((uart2p4g_sflag)||(uart2p4g_cflag==2));
}

void GroupShow_Set_Main()
{
    uart2p4g_flag_set(GroudShowMain);
}

void GroupShow_Set_Idle()
{
    uart2p4g_flag_set(GroudShowIdle);
}

int GroupShow_Action_Send(int action)
{
    DBG_PRINT("GroupShow_Action_Send action =",action);
    if(uart2p4g_sflag == 1)
    {
        RF2P4G_SEND(action|0x80);
    }
    return 0;
}

int GroupShow_Action_Make(int action)
{
    DBG_PRINT("GroupShow_Action_Make action =",action);
    uint8_t cmd[4];
    cmd[0] = 0x08;
    cmd[1] = 0x0A;
    cmd[2] = action/20;
    cmd[3] = action%20;
    tbz_app_command_send(0xFA, cmd, 4);
    return 0;
}


void uart2p4g_cdata_make(int recv)
{
    DBG_PRINT("recv =",recv);
    static int last_recv = 0;
    DBG_PRINT("last_recv =",last_recv);
    if(last_recv == recv)
        return ;
    last_recv = recv;
    if((recv>20)&&(recv<=80))
        GroupShow_Action_Make(recv);
}


void robot_2p4g_groupshow_make(uint8_t cmd)
{
    switch(cmd)
    {
        case 0xf9:
            if((uart2p4g_sflag == 0)&&(uart2p4g_cflag == 1))
                uart2p4g_flag_set(GroudShowSubSet);
            break;
        case 0xfa:
            if(uart2p4g_sflag == 1)
                RF2P4G_SEND(0xf9);
            break;
        case 0xfb:
            if(uart2p4g_cflag == 2)
                uart2p4g_flag_set(GroudShowIdle);
            break;
        case 0xfc:
            // 从机退出，不需要处理
            break;
        default:
            if(uart2p4g_cflag)
            {
                DBG_PRINT("CMD = ",cmd&0x7f);
                uart2p4g_cdata_make(cmd&0x7f);
            }
            break;
    }
}

void robot_groupshow_MainMode_Set(uint16_t set)
{
    if(set==1)
        GroupShow_Set_Main();
    else
        GroupShow_Set_Idle();
}

void robot_groupshow_SubMode_Set(uint16_t set)
{
    uart2p4g_showsubflag_set(set);
}

void Abote_GroupShow_AppData_Send(int tag,int sw)
{
    uint8_t data[10];
    int dsz = 0;
    data[dsz++] = 0xF4;
    data[dsz++] = 0x4F;
    data[dsz++] = 0x08;
    data[dsz++] = tag;
    data[dsz++] = sw;
    APP_SEND(data,dsz);
}

void groupshow_test(uint8_t set)
{
switch(set)
        {
        case 0:
            robot_groupshow_SubMode_Set(set);
            robot_2p4g_groupshow_make(0xfc);
            break;
        case 1:
            robot_groupshow_SubMode_Set(set);
            robot_2p4g_groupshow_make(0xf9);
            break;
        }
}

void Abote_GroupShow_AppData_Make(uint8_t data[])
{
    int back = 0;
    switch(data[0])
    {
    case 0x01:
        robot_groupshow_MainMode_Set(data[1]);
        break;
    case 0x02:
        switch(data[1])
        {
        case 0:
            robot_groupshow_SubMode_Set(data[1]);
            robot_2p4g_groupshow_make(0xfc);
            break;
        case 1:
            robot_groupshow_SubMode_Set(data[1]);
            robot_2p4g_groupshow_make(0xf9);
            break;
        }
        break;
    case 0x03:
        Abote_GroupShow_AppData_Send(data[0],data[1]);
        break;
    case 0x04:
        if(uart2p4g_sflag == 1){
                GroupShow_Action_Send(data[1]);}
        GroupShow_Action_Make(data[1]);
        break;
    case 0x0A:
        back = 10;
        robot_groupshow_action_make(data[1],data[2],data[3]);
        break;
    }
    if(back)
    {
        DBG_PRINT("program back : ",back);
        Abote_GroupShow_AppData_Send(back,0);
    }
}

