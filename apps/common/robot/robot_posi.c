#include "robot_main.h"
#include "robot_debug.h"
#include "robot_uart.h"
#include "robot_bus.h"
#include "robot_posi.h"
#include "robot_adapter.h"

#define __POSI_DEV_ID (79)
uint16_t global_distance_read = 0;
uint16_t ret = 0;
uint16_t abote_distance = 0;
uint16_t abote_irs = 0;

void robot_posi_modeset(uint16_t mode, uint8_t para[])
{
    STR_PRINT("robot_posi_modeset\r\n");
    switch(mode)
    {
    case 1://测距
        para[0] = 1;
        break;
    case 2://前进Z轴
        para[0] = 4;
        break;
    case 3://姿态判定
        para[0] = 4;
        break;
    default:
        para[0] = 0;
        break;
    }
}

void robot_posi_read(uint16_t mode)
{
    switch(mode)
    {
    case 1:
        ret = bus_readdata;
        break;
    case 2:
        ret = gyroData[2];
        break;
    case 3:
        if(gyroData[0]>=45&&gyroData[0]<=55)
            ret = 1;//站立
        if((gyroData[0]>=85&&gyroData[0]<=95)&&(gyroData[1]>=85&&gyroData[1]<=95))
            ret = 2;//前倒
        if((gyroData[0]>=85&&gyroData[0]<=95)&&(gyroData[1]>=175||gyroData[1]<=5))
            ret = 3;//后到
        break;
    }
}
uint16_t robot_posi_data_once(uint16_t debug, uint16_t mode)
{
    uint8_t para[6] = {0};
    uint8_t comm[12];
    uint8_t csz;
    BUS_FLUSH();
    robot_bus_read_flag_set(BUS_RF_ROBOT_POSI,0,20);
    robot_posi_modeset(mode, para);
    csz = robot_bus_command_make(__POSI_DEV_ID, 0x02, 0x02, 1, para, comm);
    BUS_WRITE(comm, csz);
    ROBOT_DELAY_MS(2);
    while (BUS_READING) // UART_INPUT_HANDLE 内含有超时判断
    {
        robot_bus_data_read_once();
        ROBOT_DELAY_MS(2);
    }
    if (bus_readid != __POSI_DEV_ID)
        bus_readdata = 0;
    if (bus_readflag != BUS_RF_END)
        bus_readdata = 0;
    robot_posi_read(mode);
    if (debug)
    {
        DBG_PRINT("bus read status = ", bus_readflag);
        DBG_PRINT("bus read id = ", bus_readid);
        DBG_PRINT("bus read data = ", bus_readdata);
        DBG_PRINT("Gyro X = ", gyroData[0]);
        DBG_PRINT("Gyro Y = ", gyroData[1]);
        DBG_PRINT("Gyro Z = ", gyroData[2]);
    }
    return ret;
}


uint16_t robot_program_get_robot_irdata(uint8_t*data1,uint8_t*data)
{
    int abote_irs_read = 0;
    abote_irs_read = robot_posi_data_once(0, 1);
    if(abote_irs_read>20)
        abote_irs = 2;
    else
        abote_irs = 1;
    return abote_irs;
}

uint16_t robot_program_get_distance(uint8_t* data)
{
    abote_distance= robot_posi_data_once(0,1);
    return abote_distance;
}
