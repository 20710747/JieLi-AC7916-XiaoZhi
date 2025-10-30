#include "robot_main.h"
#include "robot_debug.h"
#include "robot_config.h"
#include "robot_adapter.h"
#include "robot_bus.h"

/*
专门用于控制9g舵机的50HZ PWM
高电平有效范围在500-2500us
*/

#if 0
#define PWM_PORT (JL_PORTC->OUT)
#define PWM1_BIT (0x0001)
#define PWM2_BIT (0x0002)
#define PWM3_BIT (0x0020)
#define PWM4_BIT (0x0040)
#define PWM_CLR_BIT (0xFF9C)

const int pwm_io_list[]={IO_PORTC_00,IO_PORTC_01,IO_PORTC_05,IO_PORTC_06};
int pwm_duty1 = 0;//1500us
int pwm_duty2 = 0;
int pwm_duty3 = 0;
int pwm_duty4 = 0;
u16 pwm_data = 0;
int pwm_time = 0;
int pwm_all = 500;

void robot_pwm_isr(void *priv)
{
    pwm_data = 0;
    pwm_data += (pwm_time<pwm_duty1)?(PWM1_BIT):(0);
    pwm_data += (pwm_time<pwm_duty2)?(PWM2_BIT):(0);
    pwm_data += (pwm_time<pwm_duty3)?(PWM3_BIT):(0);
    pwm_data += (pwm_time<pwm_duty4)?(PWM4_BIT):(0);
    PWM_PORT |= pwm_data;
    pwm_time--;
    if(pwm_time<=0)
    {
        PWM_PORT &= PWM_CLR_BIT;
        pwm_time = pwm_all;
    }
}

void robot_pwm_init()
{
    int i;
    for(i=0;i<4;i++)
    {
        gpio_direction_output(pwm_io_list[i],0);
        gpio_set_pull_down(pwm_io_list[i],0);
        gpio_set_pull_up(pwm_io_list[i],0);
        gpio_set_die(pwm_io_list[i],1);

    }
    pwm_time = 0;
    sys_usec_timer_add(NULL,robot_pwm_isr,40,1,0);
}

void robot_pwm_set(int idx,int duty)
{
    switch(idx)
    {
    case 1:
        pwm_duty1 = duty;
        break;
    case 2:
        pwm_duty2 = duty;
        break;
    case 3:
        pwm_duty3 = duty;
        break;
    case 4:
        pwm_duty4 = duty;
        break;
    }
}

void robot_pwm_setall(int duty[])
{
    pwm_duty1 = duty[0]/40;
    pwm_duty2 = duty[1]/40;
    pwm_duty3 = duty[2]/40;
    pwm_duty4 = duty[3]/40;
    DBG2_PRINT("pwm_duty12 = ",pwm_duty1,pwm_duty2);
    DBG2_PRINT("pwm_duty34 = ",pwm_duty3,pwm_duty4);
}

uint8_t eye_para[20];
uint8_t eye_comm[40];
int eye_csz;

void robot_pwm_face_send(int duty[])
{
    //robot_bus_command_make
    int i,j;

    uint8_t eye_csz;

    for(i=0;i<8;i++)
    {
        #if 0
        j = i;
        #else
        j = (i<4)?(i+4):(i-4);
        #endif // 0
        eye_para[i*2] = duty[j]/100;
        eye_para[i*2+1] = duty[j]%100;
    }
    eye_csz = robot_bus_command_make(0x45,0x03,-1,16,eye_para,eye_comm);
    //HEXS_PRINT("BUS SEND>>",eye_comm,eye_csz);
    BUS_WRITE(eye_comm,eye_csz);
    ROBOT_DELAY_MS(10);
}

void robot_pwm_ts15_send(int data[])
{
    ROBOT_DELAY_MS(20);
    TS15_IDSTimeSend(data[0]);
    ROBOT_DELAY_MS(5);
    TS15_IDSPosiSend_asyn2(&data[1],3);
    ROBOT_DELAY_MS(5);
    TS15_asynMove_Set();
}
#endif

