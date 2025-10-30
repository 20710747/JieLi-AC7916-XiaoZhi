#ifndef __ROBOT_BUS_H__
#define __ROBOT_BUS_H__


//空闲
#define BUS_RF_IDLE    (0x00)
//结束
#define BUS_RF_END     (0x01)
//超时
#define BUS_RF_TIMEOUT (0x02)
//错误
#define BUS_RF_ERR     (0x03)
//运行状态
#define BUS_RF_RUNNING (0x10)
//搜索
#define BUS_RF_TS15_SEARCH  (0x11)
//位置
//自定义
#define BUS_RF_TS15_USER    (0x12)

#define BUS_RF_TS15_POSI    (0x21)
//温度
#define BUS_RF_TS15_TEMP    (0x22)
//电流
#define BUS_RF_TS15_CURR    (0x23)
//位置原始信息
#define BUS_RF_TS15_RAWPOSI  (0x24)
//位置信息
#define BUS_RF_ROBOT_POSI    (0x4f)
//测距数据
#define BUS_RF_ROBOT_OBST    (0x4f)
//陀螺仪
#define BUS_RF_ROBOT_GYRO    (0x4f)
//按键
#define BUS_RF_ROBOT_KEY     (0x7f)
//9g舵机
#define BUS_RF_TS02_POSI    (0x61)


//0x40-0x70 保留用于传感器
//透传模式
#define BUS_RF_TRAN    (0x7F)

extern uint16_t bus_readflag;
extern uint16_t bus_readid;
extern uint16_t bus_readdata;
extern uint8_t gyroData[3];

#define BUS_READING (bus_readflag>BUS_RF_RUNNING)

int32_t robot_bus_command_make(int32_t id,int32_t cmd,int32_t add,int32_t paralength,uint8_t *para,uint8_t output[]);
void robot_bus_data_read_once(void);
void robot_bus_read_flag_set(uint16_t flag,uint16_t length,uint16_t timeout);
int32_t robot_bus_data_reply(uint8_t *data,uint32_t dsz,uint16_t *id,uint16_t *err,uint16_t *plen,uint8_t *para);
#endif // __ROBOT_BUS_H__
