#include "robot_main.h"
#include "robot_debug.h"
#include "robot_adapter.h"
#include "robot_bus.h"
#include "robot_uart.h"

uint8_t bus_readbuff[32];
uint8_t gyroData[3];
uint16_t bus_readsz=0;
uint16_t bus_readid = 0;
uint16_t bus_readdata = 0;
uint16_t bus_readposi = 0;
uint16_t bus_readtemp = 0;
uint16_t bus_readcurr = 0;
uint16_t bus_readflag = 0;
uint16_t bus_readtime = 0;
uint16_t bus_readlen = 0;
uint16_t bus_read_timeout = 0;

int32_t robot_bus_command_make(int32_t id,int32_t cmd,int32_t add,int32_t paralength,uint8_t *para,uint8_t output[])
{
    int j,k;
    unsigned char  cs = 0;
    k = 0;
    output[k++] = 0xFF;//0 head1
    output[k++] = 0xFF;//1 head2
    output[k++] = id;//2 id
    if(add==-1)
        output[k++] = paralength+2;
    else
        output[k++] = paralength+3;
    output[k++] = cmd;
    if(add!=-1)
        output[k++] = add;
    for(j=0;j<paralength;j++)
        output[k++] = para[j];
    for(j=2;j<k;j++)
        cs += output[j];
    cs ^= 0xFF;
    if(cs==0xff)
        cs = 0x00;
    output[k++] = cs;
    return k;
}

void robot_bus_read_flag_set(uint16_t flag,uint16_t length,uint16_t timeout)
{
    bus_readsz = 0;
    bus_readtime = 0;
    bus_readflag = flag;
    bus_readlen = length;
    bus_read_timeout = timeout;
}

//读取函数只处理了正常情况，没有处理数据异常，即ERR情况，后续需补上
void robot_bus_data_read_once()
{
    uint8_t buff[32];
    int i;
    int buffsz;

    if(!BUS_READING)
        return;

    buffsz = BUS_READ(buff,31);

    if(buffsz>0)
    {
        for(i=0;i<buffsz;i++)
        {
            bus_readbuff[bus_readsz++] = buff[i];
        }
        bus_readtime = 0;
    }
    if(bus_readflag == BUS_RF_TS15_SEARCH)//搜索
    {
        if(bus_readsz>=6)
        {
            bus_readid = bus_readbuff[2];
            bus_readflag = BUS_RF_END;
            bus_readsz = 0;
        }
    }
    else if(bus_readflag == BUS_RF_ROBOT_KEY)
    {
        if(bus_readsz==7)
        {
            bus_readid = bus_readbuff[2];
            //DBG_PRINT("search id = ",bus_readid);
            bus_readdata = bus_readbuff[5];
            bus_readflag = BUS_RF_END;
            bus_readsz = 0;
        }
    }
    else if((bus_readflag >= BUS_RF_TS15_POSI)&&(bus_readflag <= BUS_RF_TS15_CURR))//位置，电流，温度等
    {
        if((bus_readsz>=8)&&(bus_readbuff[0]==0xff)&&(bus_readbuff[1]==0xf5))
        {
            bus_readid = bus_readbuff[2];
            DBG_PRINT("search id = ",bus_readid);
            bus_readdata = (bus_readbuff[5]<<8) + (bus_readbuff[6]);
            bus_readflag = BUS_RF_END;
            bus_readsz = 0;
        }
    }
    else if(bus_readflag == BUS_RF_ROBOT_GYRO)
    {

        if(bus_readsz == 10)
        {
            bus_readid = bus_readbuff[2];
            DBG_PRINT("search id = ",bus_readid);
            gyroData[0] = bus_readbuff[6]; // Gyro X
            gyroData[1] = bus_readbuff[7]; // Gyro Y
            gyroData[2] = bus_readbuff[8]; // Gyro Z
            bus_readflag = BUS_RF_END;
            bus_readsz = 0;
        }
        else if(bus_readsz == 7)
        {
            bus_readid = bus_readbuff[2];
            DBG_PRINT("search id = ",bus_readid);
            bus_readdata = (bus_readbuff[5]);
            DBG_PRINT("distance = ", bus_readdata);
            bus_readflag = BUS_RF_END;
            bus_readsz = 0;
        }
    }
    else if(bus_readflag == BUS_RF_TS02_POSI)
    {
        if(bus_readsz>=9)
        {
            bus_readid = bus_readbuff[2];
            bus_readdata = (bus_readbuff[4]<<8) + (bus_readbuff[5]);
            bus_readflag = BUS_RF_END;
            bus_readsz = 0;
            DBG2_PRINT("ts01 data read id&data = ",bus_readid,bus_readdata);
        }
    }
    else if(bus_readflag == BUS_RF_TS15_USER)
    {
        if((bus_readsz >= 6 + bus_readlen)&&(bus_readbuff[0]==0xff)&&(bus_readbuff[1]==0xf5))
        {
            bus_readid = bus_readbuff[2];
            bus_readflag = BUS_RF_END;
            bus_readsz = 0;
            DBG_PRINTF("BUS Read Status : %d",bus_readbuff[4]);
            HEXS_PRINT("BUS Read Data : ",&bus_readbuff[5],bus_readlen);
        }
    }
    if(BUS_READING)
    {
        bus_readtime++;
        if(bus_readtime > bus_read_timeout)
        {
                bus_readid = 0xFF;
                bus_readdata = 0xFF;
                bus_readtime = 0;
                bus_readflag = BUS_RF_TIMEOUT;
                DBG_PRINT("Bus Data Read Timeout", 0);
        }

    }
    //BUS_FLUSH();
    buffsz=0;
    bus_readsz=0;

}

int32_t robot_bus_data_reply(uint8_t *data,uint32_t dsz,uint16_t *id,uint16_t *err,uint16_t *plen,uint8_t *para)
{
    int i;
    int type = 0;
    if(dsz<=6)
        return 0;
    if(data[3]+4!=dsz)
        return 0;
    if((data[0]==0xff)&&(data[1]==0xf5))
        type = 1;
    else if((data[0]==0xff)&&(data[1]==0xf7))
        type = 2;
    else
        type = 0;
    if(!type)
        return type;
    //cs check
    *id = data[2];
    *err = data[4];
    *plen = data[3]-2;
    for(i=0;i<*plen;i++)
        para[i] = data[5+i];
    return type;
}
