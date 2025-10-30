#ifndef __ROBOT_UART_H__
#define __ROBOT_UART_H__

enum
{
    UART_DBG_OFF = 0,//���ߵ��Թر�
    UART_DBG_ON  = 1,//���ߵ��Դ�
    UART_DBG_ON_LOCK = 2,//���ߵ��Դ������������ù���Ч��
};

enum
{
    BUS_DIR_RXD = 0,//��������Ϊ����
    BUS_DIR_TXD = 1,//��������Ϊ���
};

enum
{
    BUS_DIRMODE_AUTO = 0,//�Զ��л�
    BUS_DIRMODE_MANUAL = 1,//�ֶ��л��������ýӿ��л�
    BUS_DIRMODE_OUTPUT = 2,//���ģʽ���޷��л�
    BUS_DIRMODE_CLOSE = 3,//�ر�
};

enum
{
    RF2P4G_MODE_SET = 0,
    RF2P4G_MODE_RW = 1,
};

#define MAIN_WRITE(D,S) robot_uart_main_write(D,S)
#define MAIN_READ(D,S) robot_uart_main_read(D,S)


#define BUS_SETIN() robot_uart_bus_direct_set(BUS_DIR_RXD)
#define BUS_SETOUT() robot_uart_bus_direct_set(BUS_DIR_TXD)

#define BUS_WRITE(D,S) robot_uart_bus_write(D,S)
#define BUS_READ(D,S) robot_uart_bus_read(D,S)
#define BUS_FLUSH() robot_uart_bus_flush()
#define BUS_TS15ACK_READ(D,S,T) robot_uart_bus_ts15ack_read(D,S,T)

#define RF2P4G_WRITE(D,S) robot_uart_rf2p4g_write(D,S)
#define RF2P4G_READ(D,S) robot_uart_rf2p4g_read(D,S)

#define RF2P4G_SEND(D) robot_uart_rf2p4g_send(D)
#define RF2P4G_RECV() robot_uart_rf2p4g_recv()

#define RF2P4G_SET_CHANNEL(CH,CFG) robot_uart_rf2p4g_channel_set(CH,CFG)
#define RF2P4G_GET_CHANNEL() robot_uart_rf2p4g_channel_get()
#endif

