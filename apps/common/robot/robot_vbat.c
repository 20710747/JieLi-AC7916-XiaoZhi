#include "robot_main.h"
#include "robot_debug.h"
#include "robot_control.h"
#include "robot_vbat.h"
#include "robot_adapter.h"

#include "../../../include_lib/driver/cpu/wl82/asm/adc_api.h"

#define ROBOT_VBAT_DET_CH 0x0b
#define ROBOT_VBAT_APP_LEVEL 10
#define ROBOT_VBATLOW_DET_TIME_SET 10000//ms
#define ROBOT_DCIN_DETIO IO_PORTH_03
#define ROBOT_DCIN_DETVAL 1

int robot_vbat_level;
int robot_vbat_adjust_level;
int robot_vbat_low_level = 600;
int robot_vBatRecovery_det = 0;
int robot_vbatlow_time = 0;
int robot_vbatlow_det_time = 0;
int Robot_vBatLow_Flag = 0;

int robot_vbat_level_app_list[ROBOT_VBAT_APP_LEVEL]=
{
    500,550,600,650,700,
    750,800,850,900,1000,
};

int Robot_vBatLow_Flag_Get()
{
    return Robot_vBatLow_Flag;
}
int robot_vbat_adjust_level_get()
{
    //在flash中读取
    return 0;
}

int robot_vbat_adjust_level_set(int adjust)
{
    robot_vbat_adjust_level = adjust;

}
int robot_vbat_level_set(int adc)
{
    robot_vbat_level = adc;
}

void robot_vbat_sample_isr(void *priv)
{
    int adc = adc_get_value(AD_CH_PB07);

    if (adc == 0) {

        //STR_PRINT("Error: ADC read zero, check sensor connection or configuration!\r\n");
        return;
    }

    robot_vbat_level_set(adc);

    //DBG_PRINT("Battery Level Set to: ", robot_vbat_level);
}


int robot_vbat_raw_data_get()
{
    DBG_PRINT("robot_vbat_level =",robot_vbat_level);
    return robot_vbat_level;
}

int robot_vbat_app_level_get()
{
    int i;
    for(i=0;i<ROBOT_VBAT_APP_LEVEL;i++)
    {
        if(robot_vbat_level<robot_vbat_level_app_list[i])
            break;
    }
    DBG_PRINT("robot_vbat_app_level_get =",i);
    return i;
}
//判断是否有不适宜检测电量的状态,主要就是判断动作，声音状态
int robot_vbat_status_judge()
{
    return 0;

}


void robot_vbat_once()
{
    if(robot_vbat_level<robot_vbat_low_level)
    {
        if(robot_vbat_status_judge())
            return ;
        if(robot_vbatlow_time==0)
        {
            robot_vbatlow_time = ROBOT_GETTIME_MS();
            robot_vbatlow_det_time = 0;
        }
        else
        {
            robot_vbatlow_det_time = ROBOT_GETTIME_MS()-robot_vbatlow_time;
            if(robot_vbatlow_det_time>ROBOT_VBATLOW_DET_TIME_SET)
            {
                if(!Robot_vBatLow_Flag)
                    DBG_PRINT("ROBOT VBAT LOW FLAG SET",1);
                Robot_vBatLow_Flag = 1;
                robot_vBatRecovery_det = 20;
            }
        }
    }
    else
    {
        if(!Robot_vBatLow_Flag)
            robot_vbatlow_time = 0;
        else
        {
            robot_vBatRecovery_det--;
            if(robot_vBatRecovery_det<0)
            {
                DBG_PRINT("ROBOT VBAT LOW FLAG CLR",0);
                Robot_vBatLow_Flag = 0;
            }

        }
    }
}




void robot_vbat_init()
{
    //gpio_set_direction(IO_PORTB_07, 1);
    //gpio_set_pull_up(IO_PORTB_07, 1);
    //gpio_set_pull_down(IO_PORTB_07, 0);
    //gpio_set_die(IO_PORTB_07, 0);
    int init = gpio_set_direction(IO_PORTB_07, 1);
    DBG_PRINT("VABAT_TSET =",init);
    adc_pmu_detect_en(AD_CH_PB07);
    adc_add_sample_ch(AD_CH_PB07);

    sys_timer_add(NULL,robot_vbat_sample_isr,500);
}
void vbat_test()
{
    int init = gpio_set_direction(IO_PORTB_07, 1);
    DBG_PRINT("VABAT_TSET =",init);
}
int robot_dcin_flag = 0;

void robot_dcin_init()
{
        gpio_set_direction(ROBOT_DCIN_DETIO, 1);
        gpio_set_pull_up(ROBOT_DCIN_DETIO, 1);
        gpio_set_pull_down(ROBOT_DCIN_DETIO, 0);
        gpio_set_die(ROBOT_DCIN_DETIO, 1);

        STR_PRINT("dcin_init SCCUSE \r\n");
}

int robot_dcin_det()
{
    int cur_dcdet = (gpio_read(ROBOT_DCIN_DETIO)==ROBOT_DCIN_DETVAL);
    int ret = 0;
    if(!cur_dcdet)
    {
        if(robot_dcin_flag)
            ret = 1;
        else
            ret = 0;
    }
    else
    {
        if(!robot_dcin_flag)
            ret = 3;
        else
            ret = 2;
    }
    robot_dcin_flag = cur_dcdet;
    return ret;
}

