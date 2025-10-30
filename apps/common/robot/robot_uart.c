#include "robot_main.h"
#include "robot_debug.h"
#include "robot_adapter.h"
#include "robot_uart.h"


#define UART_MAIN_USE     1
#define UART_BUS_USE      1
#define UART_RF2P4G_USE   1

#if 0
#if CONFIG_DEBUG_CHANEL==0
#undef UART_MAIN_USE
#define UART_MAIN_USE 0
#endif // CONFIG_DEBUG_CHANEL

#if CONFIG_DEBUG_CHANEL==1
#undef UART_BUS_USE
#define UART_BUS_USE 0
#endif // CONFIG_DEBUG_CHANEL
#endif

#if UART_MAIN_USE
#define UART_MAIN_CHANNEL "uart0"
#endif // UART_MAIN_USE

#if UART_BUS_USE
#define UART_BUS_CHANNEL  "uart1"

/*
1OE  2OE   BUS
 0    1    RXD
 1    0    TXD
*/

#define BUS_1OE IO_PORTD_03
#define BUS_2OE IO_PORTD_02

#define BUS_MODE_DEFAULT  BUS_DIRMODE_MANUAL
#define BUS_DEBUG_DEFAULT UART_DBG_OFF
#endif //UART_BUS_USE

#if UART_RF2P4G_USE
#define UART_RF2P4G_CHANNEL "uart2"

// 设置PIN:
//PIN为低(PIN = 0)，RF2.4G模组处于设置模式
//PIN为高(PIN = 1)，RF2.4G模组处于透传(收发)模式。
#define RF2P4G_SETPIN IO_PORTC_08

// 默认频段分组1
#define RF2P4G_DEFAULT_GROUP1 (0x61)
// 默认频段分组2
#define RF2P4G_DEFAULT_GROUP2 (0x16)
// 默认RF信道
#define RF2P4G_DEFAULT_RFCH   (0x66)

// 发送重复次数宏
#define RF2P4G_TX_REPEAT_COUNT 9
// 接收最小有效次数宏
#define RF2P4G_RX_MIN_VALID_COUNT 6
// 接收最大允许出现不同数字
#define RF2P4G_RX_MAX_VALID_VALUE 3
// 接收最大读取数量
#define RF2P4G_RX_MAX_READ_SZ 32

#endif // UART_RF2P4G_USE


#define ROBOT_UART_TXBUF_SIZE   0x200
#define ROBOT_UART_RXBUF_SIZE   0x200

struct uart_hdl {
    void *uart_dev;
    u8 rx_type;
    volatile u8 run_flag;
    s16 data_length;
    void *pTxBuffer;
};

#if UART_MAIN_USE
static struct uart_hdl uart_main;
#define __robot_main      (&uart_main)
static u8 uart_buf_main[ROBOT_UART_RXBUF_SIZE] __attribute__((aligned(32)));

#endif //UART_MAIN_USE

#if UART_BUS_USE
static struct uart_hdl uart_bus;
#define __robot_bus (&uart_bus)
static u8 uart_buf_bus[ROBOT_UART_RXBUF_SIZE] __attribute__((aligned(32)));

static int robot_uart_bus_debug = BUS_DEBUG_DEFAULT;
static int robot_uart_bus_dirmode = -1;
static int robot_uart_bus_dir = -1;
#define UART_BUS_DIR_SET(DIR) \
    do { \
        gpio_direction_output(BUS_1OE, (DIR) == BUS_DIR_TXD ? 1 : 0); \
        gpio_direction_output(BUS_2OE, (DIR) == BUS_DIR_TXD ? 0 : 1); \
    } while (0)

#endif // UART_BUS_USE

#if UART_RF2P4G_USE
static struct uart_hdl uart_rf2p4g;
#define __robot_rf2p4g     (&uart_rf2p4g)
static u8 uart_buf_rf2p4g[ROBOT_UART_RXBUF_SIZE] __attribute__((aligned(32)));
static int robot_uart_rf2p4g_debug = UART_DBG_OFF;
static int robot_uart_rf2p4g_channel = -1;
static int robot_uart_rf2p4g_mode = RF2P4G_MODE_RW;
#endif // UART_RF2P4G_USE

//-------------------  串口句柄初始化 ------------------------
//uart init
void robot_uart_handle_init(struct uart_hdl *pUartHDL,char uartid[],u8 *buff,int bufflen)
{
    u8 *recv_buf;
    pUartHDL->uart_dev = dev_open(uartid, NULL);

    if (!pUartHDL->uart_dev)\
    {
        printf("robot main uart openerr !!!\n");
        return ;
    }

    /* 1 . 设置接收数据地址 */
    dev_ioctl(pUartHDL->uart_dev, UART_SET_CIRCULAR_BUFF_ADDR, (int)buff);

    /* 2 . 设置接收数据地址长度 */
    dev_ioctl(pUartHDL->uart_dev, UART_SET_CIRCULAR_BUFF_LENTH, bufflen);

    /* 3 . 设置接收数据为阻塞方式,需要非阻塞可以去掉,建议加上超时设置 */
    dev_ioctl(pUartHDL->uart_dev, UART_SET_RECV_BLOCK, 1);

    u32 parm = 20;//接收函数的超时时间要比发送函数超时时间小
    dev_ioctl(pUartHDL->uart_dev, UART_SET_RECV_TIMEOUT, (u32)parm);
    pUartHDL->run_flag=1;
    //thread_fork("uart_recv_test_task", 20, 2048, 0, 0, uart_recv_test_task, NULL);

    /* 4 . 使能特殊串口 */
    dev_ioctl(pUartHDL->uart_dev, UART_START, 0);
}

//-------------------  主串口 ------------------------



void robot_uart_main_write(void *data,uint32_t sz)
{
    if(__robot_main->uart_dev && data && sz)
        dev_write(__robot_main->uart_dev, data, sz);
}

int robot_uart_main_read(void *data,uint32_t sz)
{
    if((data==NULL)||(sz==0))
        return 0;
    if(sz > ROBOT_UART_RXBUF_SIZE)
        sz = ROBOT_UART_RXBUF_SIZE;
    if(__robot_main->uart_dev)
        __robot_main->data_length = dev_read(__robot_main->uart_dev, data, sz);
    else
        __robot_main->data_length = 0;
    if(__robot_main->data_length<0)
        __robot_main->data_length = 0;
    return __robot_main->data_length;
}

void robot_uart_main_init()
{
    DBG_PRINTF("robot_uart_main_init use = %d",UART_MAIN_USE);
    #if UART_MAIN_USE
    robot_uart_handle_init(__robot_main,UART_MAIN_CHANNEL,uart_buf_main,sizeof(uart_buf_main));
    //MAIN_WRITE("123\r\n",5);
    //MAIN_WRITE("456\r\n",5);
    #else
    __robot_main->uart_dev = NULL;
    #endif
}

//----------------------------------------------------




//------------------- 总线串口 ----------------------


void robot_uart_bus_debug_set(int debug)
{
    if(robot_uart_bus_debug!=UART_DBG_ON_LOCK)//设置成BUS_DBG_ON_LOCK之后，再也不能设置回来咯。
        robot_uart_bus_debug = debug;
}

int robot_uart_bus_debug_get()
{
    return robot_uart_bus_debug;
}
//0 in 1 out
void robot_uart_bus_direct_set(int dir)
{
    if(robot_uart_bus_dirmode == BUS_DIRMODE_OUTPUT)
        dir = BUS_DIR_TXD;
    else if(robot_uart_bus_dirmode == BUS_DIRMODE_CLOSE)
        dir = BUS_DIR_RXD;
    robot_uart_bus_dir = dir;
    DBG_PRINTF("bus dir = %d",robot_uart_bus_dir);
    UART_BUS_DIR_SET(dir);
}

int robot_uart_bus_direct_get()
{
    return robot_uart_bus_dir;
}


void robot_uart_bus_read_end()
{
    robot_uart_bus_direct_set(BUS_DIR_TXD);
}

void robot_uart_bus_dirmode_set(int mode)
{
    robot_uart_bus_dirmode = mode;
    if((robot_uart_bus_dirmode == BUS_DIRMODE_AUTO)||(robot_uart_bus_dirmode == BUS_DIRMODE_CLOSE))
        robot_uart_bus_direct_set(BUS_DIR_RXD);
    else
        robot_uart_bus_direct_set(BUS_DIR_TXD);
}

int robot_uart_bus_dirmode_get()
{
    return robot_uart_bus_dirmode;
}


void robot_uart_bus_write(void *data,uint32_t sz)
{
    if(robot_uart_bus_dirmode == BUS_DIRMODE_AUTO)
        UART_BUS_DIR_SET(BUS_DIR_TXD);
    if(__robot_bus->uart_dev && data && sz)
    {
        if(robot_uart_bus_debug)
            HEXS_PRINT("BUS OUT",data,sz);
        if(robot_uart_bus_dir==BUS_DIR_TXD)
            dev_write(__robot_bus->uart_dev, data, sz);
    }
    if(robot_uart_bus_dirmode == BUS_DIRMODE_AUTO)
        UART_BUS_DIR_SET(BUS_DIR_RXD);
}

int robot_uart_bus_read(void *buff,uint32_t sz)
{
    int read_sz;
    if((buff==NULL)||(sz==0))
        return 0;
    if(sz > ROBOT_UART_RXBUF_SIZE)
        sz = ROBOT_UART_RXBUF_SIZE;
    if(__robot_bus->uart_dev)
        __robot_bus->data_length = dev_read(__robot_bus->uart_dev, buff, sz);
    else
        __robot_bus->data_length = 0;

    if(__robot_bus->data_length>0)
    {
        if(robot_uart_bus_debug)
            HEXS_PRINT("BUS IN: ",(uint8_t*)buff,__robot_bus->data_length);
    }
    return __robot_bus->data_length;
}

void robot_uart_bus_flush()
{
    u8 ClrBuffer[32];
    //DBG_PRINT("robot_bus_data_clr ",__robot_bus);
    if(__robot_bus->uart_dev)
    {
        do
        {
            __robot_bus->data_length = dev_read(__robot_bus->uart_dev, ClrBuffer, 31);
        }while(__robot_bus->data_length>0);
        dev_ioctl(__robot_bus->uart_dev,UART_FLUSH,0);
    }
    else
    {
        __robot_bus->data_length = 0;
    }
}

/*
TS15回码：
01，过压保护，第零位
04，过温保护，第一位
40，堵转保护，第五位。
*/
int robot_uart_bus_ts15ack_read(uint16_t ts15vis[][2],int sz,int timeout)
{
    int i;
    int k;
    u8 buff[40];
    int bsz;
    int rsz = 0;
    u32 curtime = ROBOT_GETTIME_MS();

    while(rsz < sz)
    {
        u32 nowtime = ROBOT_GETTIME_MS();
        if(nowtime - curtime > (u32)timeout) {
            break;
        }
        if((bsz = robot_bus_data_read(buff,40)) < 0) {
            if(bsz == -2) {
                ROBOT_DELAY_US(100);
                continue;
            }
            break;
        }

        for(i=0;i<bsz;i++)
        {
            if((buff[i]>=1)&&(buff[i]<=17))
            {
                int id = buff[i];
                if(ts15vis[id][0] == 0)
                {
                    ts15vis[id][0] = 1;
                    rsz++;
                }
            }
        }
    }
    return rsz;
}


void robot_uart_bus_init()
{
    DBG_PRINTF("robot_uart_bus_init use = %d",UART_BUS_USE);
    #if UART_BUS_USE
    robot_uart_handle_init(__robot_bus,UART_BUS_CHANNEL,uart_buf_bus,sizeof(uart_buf_bus));
    robot_uart_bus_dirmode_set(BUS_MODE_DEFAULT);
    #else
        __robot_bus->uart_dev = NULL;
    #if CONFIG_DEBUG_CHANEL==1
        robot_uart_bus_dirmode_set(BUS_DIRMODE_OUTPUT);
        robot_bus_direct_set(BUS_DIR_TXD);
    #endif
    #endif
}
//---------------------------------------------------

//------------------- 2.4G串口 -----------------------

void robot_uart_rf2p4g_debug_set(int debug)
{
    if(robot_uart_rf2p4g_debug!=UART_DBG_ON_LOCK)//设置成BUS_DBG_ON_LOCK之后，再也不能设置回来咯。
        robot_uart_rf2p4g_debug = debug;
}

int robot_uart_rf2p4g_debug_get()
{
    return robot_uart_rf2p4g_debug;
}

void robot_uart_rf2p4g_mode_set(int mode)
{
    robot_uart_rf2p4g_mode = mode;
    gpio_direction_output(RF2P4G_SETPIN,mode);
}

int robot_uart_rf2p4g_mode_get()
{
    return robot_uart_rf2p4g_mode;
}

int robot_uart_rf2p4g_online()
{
    return (__robot_rf2p4g->uart_dev)?(1):(0);
}

void robot_uart_rf2p4g_write(void* data,uint32_t sz)
{
    if(__robot_rf2p4g->uart_dev && data && sz)
    {
        if(robot_uart_rf2p4g_debug)
        {
            HEXS_PRINT("RF2P4G OUT: ",data,sz);
        }
        dev_write(__robot_rf2p4g->uart_dev, data, sz);
    }
}

int16_t robot_uart_rf2p4g_read(void *buff,uint16_t sz)
{
    if(__robot_rf2p4g)
        __robot_rf2p4g->data_length = dev_read(__robot_rf2p4g->uart_dev, buff, sz);
    else
        __robot_rf2p4g->data_length = 0;
    if((robot_uart_rf2p4g_debug)&&(__robot_rf2p4g->data_length>0))
    {
        HEXS_PRINT("RF2P4G IN: ",buff,__robot_rf2p4g->data_length);
    }

    return __robot_rf2p4g->data_length;
}


void robot_uart_rf2p4g_send(uint8_t data) {
    // 创建重复数据缓冲区
    uint8_t tx_buffer[RF2P4G_TX_REPEAT_COUNT];

    // 填充重复数据
    for (int i = 0; i < RF2P4G_TX_REPEAT_COUNT; i++) {
        tx_buffer[i] = data;
    }

    // 发送数据
    robot_uart_rf2p4g_write(tx_buffer, RF2P4G_TX_REPEAT_COUNT);
}


uint8_t robot_uart_rf2p4g_recv(void) {
    // 接收数据缓冲区
    uint8_t rx_buffer[RF2P4G_RX_MAX_READ_SZ];

    // 读取数据
    int bytes_read = robot_uart_rf2p4g_read(rx_buffer, RF2P4G_RX_MAX_READ_SZ);

    if (bytes_read <= 0) {
        return 0; // 没有接收到数据
    }

    // 使用结构体记录值和计数
    typedef struct {
        uint8_t value;
        uint8_t count;
    } value_count_t;

    // 分配空间 - 最多记录RF2P4G_RX_MAX_VALID_VALUE个不同值
    value_count_t value_counts[RF2P4G_RX_MAX_VALID_VALUE] = {{0}};
    int unique_count = 0;

    // 统计每个数值的出现次数
    for (int i = 0; i < bytes_read; i++) {
        uint8_t current = rx_buffer[i];
        int found = 0;

        // 查找是否已记录该值
        for (int j = 0; j < unique_count; j++) {
            if (value_counts[j].value == current) {
                value_counts[j].count++;
                found = 1;
                break;
            }
        }

        // 如果未找到且还有空间，添加新值
        if (!found) {
            if (unique_count < RF2P4G_RX_MAX_VALID_VALUE) {
                value_counts[unique_count].value = current;
                value_counts[unique_count].count = 1;
                unique_count++;
            } else {
                // 超过最大允许的不同值数量，认为数据无效
                return 0;
            }
        }
    }

    // 找出出现次数最多的数值
    uint8_t most_frequent = 0;
    int max_count = 0;
    for (int i = 0; i < unique_count; i++) {
        if (value_counts[i].count > max_count) {
            max_count = value_counts[i].count;
            most_frequent = value_counts[i].value;
        }
    }

    // 检查是否达到最小有效次数
    return (max_count >= RF2P4G_RX_MIN_VALID_COUNT) ? most_frequent : 0;
}

void robot_uart_rf2p4g_channel_set(int channel,uint8_t *cfg)
{
    uint8_t cs = 0;
    unsigned char init_data[] =
    {
        0xAA,0x5A,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x04,0x00,0x62,
        0x00,0x00,0x00,0x12,0x00,0x00,
    };
    if(robot_uart_rf2p4g_channel == channel)
        return ;
    INFO_PRINTF("robot_uart_rf2p4g_channel_set %d",channel);
    robot_uart_rf2p4g_channel = channel;
    if((channel==1)&&(cfg==NULL))
    {
        channel = 0;
    }
    switch(channel)
    {
    case 0:
        init_data[4] = RF2P4G_DEFAULT_GROUP1;
        init_data[5] = RF2P4G_DEFAULT_GROUP2;
        init_data[11] = RF2P4G_DEFAULT_RFCH;
        break;
    case 1:
        init_data[4] = cfg[0];
        init_data[5] = cfg[1];
        init_data[11] = cfg[2];
        break;
    default:
        init_data[4] = channel-1;
        init_data[5] = channel-1;
        init_data[11] = 0x50+channel-1;
        break;
    }
    cs = 0x1A+init_data[4]+init_data[5]+init_data[11];
    init_data[17] = cs&0xFF;
    if(!robot_uart_rf2p4g_online())
        return ;
    robot_uart_rf2p4g_mode_set(RF2P4G_MODE_SET);
    //DBG_PRINTF("robot_uart_rf2p4g_channel_step ",1);
    ROBOT_DELAY_MS(100);
    robot_uart_rf2p4g_write(init_data,18);
    //DBG_PRINTF("robot_uart_rf2p4g_channel_step ",2);
    ROBOT_DELAY_MS(100);
    robot_uart_rf2p4g_mode_set(RF2P4G_MODE_RW);
    //DBG_PRINTF("robot_uart_rf2p4g_channel_step ",3);
    ROBOT_DELAY_MS(100);
}

int robot_uart_rf2p4g_channel_get()
{
    return robot_uart_rf2p4g_channel;
}

void robot_uart_rf2p4g_init()
{
    DBG_PRINTF("robot_uart_rf2p4g_init use = %d",UART_RF2P4G_USE);
    #if UART_RF2P4G_USE
    robot_uart_handle_init(__robot_rf2p4g,UART_RF2P4G_CHANNEL,uart_buf_rf2p4g,sizeof(uart_buf_rf2p4g));
    //DBG_PRINTF("robot_uart_rf2p4g_init step = %d",1);
    robot_uart_rf2p4g_channel_set(0,NULL);
    //DBG_PRINTF("robot_uart_rf2p4g_init step = %d",2);
    robot_uart_rf2p4g_mode_set(RF2P4G_MODE_RW);
    //DBG_PRINTF("robot_uart_rf2p4g_init step = %d",3);
    #endif // UART_RF2P4G_USE
}

//----------------------------------------------------





// --------------------- 初始化 -----------------------
void robot_uart_init()
{
    //DBG_PRINTF("robot_uart_init %d",0);
    robot_uart_main_init();
    robot_uart_bus_init();
    robot_uart_rf2p4g_init();
    DBG_PRINTF("robot_uart_init ok!");
}
//---------------------------------------------------------
