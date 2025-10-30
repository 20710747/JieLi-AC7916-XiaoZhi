#include "robot_main.h"
#include "robot_debug.h"
#include "robot_light.h"
#include "robot_adapter.h"

//Head Light Sender
#define LIGHT_SEND_IO IO_PORTB_06
#define LIGHT_SEND_IO_HIGH gpio_direction_output(IO_PORTB_06,1)
#define LIGHT_SEND_IO_LOW gpio_direction_output(IO_PORTB_06,0)
#define LIGHT_SEND_LEAD_CLICK 4

#define ONE_HIGH_CLICK 3
#define ONE_LOW_CLICK 1
#define ZERO_HIGH_CLICK 1
#define ZERO_LOW_CLICK 3

uint16_t light_ctl_input = 0;
uint16_t light_ctl_send = 0;
uint16_t light_send_bit = 0;
uint16_t light_time_left = 0;
uint16_t light_send_step = 0;


#if 1
//模拟UART
void Uart_Delay_Us(int time)
{
    delay_us(time);
}

void IR_SendByte(uint16_t val)//发送bit位
{
    u16 i;

    LIGHT_SEND_IO_LOW;
    Uart_Delay_Us(833);
    Uart_Delay_Us(833);

    DBG_PRINT("VAL =",val);
    for(i=0;i<16;i++)
    {
         LIGHT_SEND_IO_HIGH;
         Uart_Delay_Us(833);
         if(val&0x8000)
         {
              Uart_Delay_Us(833);
              Uart_Delay_Us(833);
              Uart_Delay_Us(833);
         }
         LIGHT_SEND_IO_LOW;
         Uart_Delay_Us(833);
         if(!(val&0x8000))
         {
              Uart_Delay_Us(833);
              Uart_Delay_Us(833);
              Uart_Delay_Us(833);
         }
         val<<=1;
     }
     LIGHT_SEND_IO_HIGH;
     Uart_Delay_Us(833);
}

void light_ctl_set_rgb(uint16_t mode,uint16_t r4,uint16_t g4,uint16_t b4)
{
    light_ctl_input = (mode<<12)+(r4<<8)+(g4<<4)+(b4);
    printf("head_ctl_set >> %d %d %d %d\r\n",mode,r4,g4,b4);
    DBG_PRINT("mode = ",mode);
    DBG_PRINT("r4 = ",r4);
    DBG_PRINT("g4 = ",g4);
    DBG_PRINT("b4 = ",b4);
    IR_SendByte(light_ctl_input);
}

uint32_t light_clr_value[70]=
{
    // grays
    0xffffffff, 0xffcccccc, 0xffc0c0c0, 0xff999999, 0xff666666, 0xff333333, 0xff000000,
    // reds
    0xffffcccc, 0xffff6666, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000,
    // oranges
    0xffffcc99, 0xffff9966, 0xffff9900, 0xffff6600, 0xffcc6600, 0xff993300, 0xff663300,
    // yellows
    0xffffff99, 0xffffff66, 0xffffcc66, 0xffffcc33, 0xffcc9933, 0xff996633, 0xff663333,
    // olives
    0xffffffcc, 0xffffff33, 0xffffff00, 0xffffcc00, 0xff999900, 0xff666600, 0xff333300,
    // greens
    0xff99ff99, 0xff66ff99, 0xff33ff33, 0xff33cc00, 0xff009900, 0xff006600, 0xff003300,
    // turquoises
    0xff99ffff, 0xff33ffff, 0xff66cccc, 0xff00cccc, 0xff339999, 0xff336666, 0xff003333,
    // blues
    0xffccffff, 0xff66ffff, 0xff33ccff, 0xff3366ff, 0xff3333ff, 0xff000099, 0xff000066,
    // purples
    0xffccccff, 0xff9999ff, 0xff6666cc, 0xff6633ff, 0xff6600cc, 0xff333399, 0xff330099,
    // violets
    0xffffccff, 0xffff99ff, 0xffcc66cc, 0xffcc33cc, 0xff993399, 0xff663366, 0xff330033
};

void light_ctl_set_col(int mode,int colour)
{
    int r,g,b;
    DBG_PRINT("mode = ",mode);
    DBG_PRINT("colour = ",colour);
    DBG_PRINT_UINT32("light_clr_value[colour]=",light_clr_value[colour]);
    r = (light_clr_value[colour]>>20)&(0x0f);
    g = (light_clr_value[colour]>>12)&(0x0f);
    b = (light_clr_value[colour]>>4)&(0x0f);
    light_ctl_set_rgb(mode,r,g,b);

}

void light_ctl_init()
{
    LIGHT_SEND_IO_HIGH;

    //sys_timer_add(NULL,TbzLightCtlSenderIsr,1);
}
#endif
