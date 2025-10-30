#include "robot_main.h"
#include "robot_control.h"
#include "robot_action.h"
#include "robot_irfollow.h"
#include "robot_debug.h"
#include "robot_adapter.h"

#include "key/irkey.h"
#include "asm/pwm.h"

int robot_irfollow_switch = 0;
int robot_irobs_touch_time = 0;
int robot_irobs_break_time = 0;
int robot_irobs_switch = 0;
int robot_irfollow_idle_time = 0;

//与红外相关的配置，直接写到这里
#define IR_POSI_NUM 4
#define IR_RECV_TIMEOUT 20
#define IR_IO_ST IO_PORTC_02
extern struct irkey_platform_data irkey_data;
extern void ir_flag_clean();

const int ir_io_offset[IR_POSI_NUM]={3,2,1,0};//左右前后

int ir_io_idx = 0;
int ir_rec_timer = 0;
int ir_finish_flag = 0;
int ir_finish_state = 0;
int ir_rec_state = 0;

int ir_follow_result_access = 0;
int ir_follow_result_direction = 0;
int ir_follow_adjust_timer = 0;
uint16_t robot_irfllowe_get_result(uint16_t *paccess,uint16_t *pdir)
{
    if(ir_follow_result_access)
    {
        if(pdir)    *pdir = ir_follow_result_direction;
        if(paccess) *paccess = ir_follow_result_access;
        return 1;
    }
    else
    {
        return 0;
    }
}

void robot_irfllowe_clean_result()
{
    ir_follow_result_access = 0;
    ir_follow_result_direction = 0;
    ir_finish_flag = 0;
}

void robot_irfollow_adjust_ok()
{
    if(robot_irfollow_switch&0x02)
    {
        STRDBG_PRINT("IR_FOLLOW_ADJUST","OK");
    }
}

void robot_ir_show(void)
{
    int show_state = ir_finish_state;
    int access = 0;
    int direction = 0;
    uint8_t cmd = 0;
    DBG_PRINT("IR_STATE =",show_state);
    show_state = show_state&0x7F;//定义后弱波为无效波。
    if(show_state&0x70)
    {
        show_state = (show_state&0x0F)|(show_state>>4);
        DBG_PRINT("IR_TOUCH",show_state);
        access = 1;
    }
    else if(show_state&0x07)
    {
        show_state = show_state&0x07;
        DBG_PRINT("IR_FOLLOW",show_state);
        access = 2;
    }
    else if(show_state&0x08)
    {
        DBG_PRINT("IR_BACK",0x08);
        access = 3;
    }
    else
    {
        DBG_PRINT("IR_NULL",0);
    }
    switch(show_state)
    {
    case 1:
    case 5:
        STR_PRINT("DIR LEFT\r\n");
        direction = 3;
        break;
    case 2:
    case 6:
        STR_PRINT("DIR RIGHT\r\n");
        direction = 4;
        break;
    case 3:
    case 4:
    case 7:
        STR_PRINT("DIR FORWARD\r\n");
        direction = 1;
        break;
    case 8:
        STR_PRINT("DIR BACKWARD\r\n");
        direction = 2;
    default:
        break;
    }
    ir_follow_result_direction = direction;
    ir_follow_result_access = access;
    if(show_state&0x03)
    {
        ir_follow_adjust_timer++;
        if(ir_follow_adjust_timer>=5)
            cmd = 0x52;
    }
    else if(show_state==0)
    {
        cmd = 0x53;
    }
    else
    {
        ir_follow_adjust_timer = 0;
    }
    //if(cmd)
    //    GS_SEND(&cmd,1);
}

void robot_ir_state_add(uint16_t type)
{
    if((ir_io_idx<0) || (ir_io_idx>=IR_POSI_NUM))
        return ;
    //if(type==0)
    //    DBG_PRINT("robot_ir_follow_add ",ir_io_idx);
    //else
    //    DBG_PRINT("robot_ir_touch_add ",ir_io_idx);
    ir_rec_state |= ((1<<ir_io_idx)<<type);
}

void robot_ir_timer_once()//20ms
{
    if(!robot_irfollow_switch)
        return ;
    ir_rec_timer--;
    if(ir_rec_timer < 0)
    {
        ir_io_idx++;
        if(ir_io_idx)
        {
            if(robot_irfollow_switch&0x02)
            {
                DBG_PRINT("state = ",ir_rec_state);
                robot_irdebug_cap_show();
            }
            robot_irdebug_cap_init();
        }
        //if(ir_io_idx+ir_rec_flag*2>=IR_IO_NUM)//如果左右能接收到，则不需要检测前后，如果前接收到，则不需要接收后
        if(ir_io_idx>=IR_POSI_NUM)//根据实际效果来调整红外策略
        {
            ir_io_idx = 0;
            ir_finish_state = ir_rec_state;
            ir_rec_state = 0;
            robot_ir_show();
            ir_finish_flag = 1;
        }
        irkey_data.port = IR_IO_ST + ir_io_offset[ir_io_idx];
        if(robot_irfollow_switch&0x02)
            DBG_PRINT("ir port set : ",irkey_data.port);
        ir_input_io_sel(irkey_data.port);
        ir_flag_clean();
        //DBG_PRINT("io sel ok",0);
        ir_rec_timer = IR_RECV_TIMEOUT;
    }
}

extern int get_time_pad();

void robot_irfollow_switch_set(uint16_t set)
{

#if IR_CAP_DEBUG
    //DBG_PRINT("ir->time_pad : ",get_time_pad());
#endif
    if(set)//开
    {
        ir_io_idx = -1;
        ir_rec_timer = 0;
        ir_rec_state = 0;
        ir_follow_adjust_timer = 0;
        robot_irfollow_idle_time = 0;
        robot_irdebug_cap_init();
        robot_irfllowe_clean_result();
        robot_irfollow_switch |= set;
//        DBG_PRINT("robot_irfollow_switch_set >>",set);
    }
    else//关
    {
        robot_irfollow_switch = 0;
        DBG_PRINT("robot_irfollow_switch_set >>",set);
    }
}

uint16_t robot_irfollow_switch_get()
{
    return robot_irfollow_switch;
}

void robot_irfollow_init()
{
    ir_io_idx = 0;
    ir_rec_timer = 0;
    ir_finish_flag = 0;
    ir_rec_state = 0;
}

void robot_irfollow_move_go()
{
    uint16_t acc,dir;
    uint16_t step = 0;
    int wait_timer = 0;
    int forwart_flag = 0;
    if(!robot_irfollow_switch)
        return ;
    if(robot_irfllowe_get_result(NULL,NULL))
    {
        if(robot_irfollow_switch&0x04)
            DBG_PRINT("IR_GET",robot_irfollow_idle_time);
        if(robot_irfollow_idle_time>=20)
        {
            robot_audio_voice_play(4);
        }
        robot_irfollow_idle_time = 0;
    }
    else
    {
        if(robot_irfollow_switch&0x04)
            DBG_PRINT("IR_IDLE",robot_irfollow_idle_time);
        if(robot_irfollow_idle_time>=40)
        {
            robot_audio_voice_play(13);
            robot_irfollow_idle_time = 0;
        }
        robot_irfollow_idle_time++;
        return ;
    }
    while(1)
    {
        if(!robot_irfollow_switch)
            break;
        if(ir_finish_flag)
        {
            DBG_PRINT("IR_NEW_GET\r\n",1);
            if(!robot_irfllowe_get_result(&acc,&dir))
                break;
            robot_irfllowe_clean_result();
            if(dir==1)
            {
                forwart_flag = 1;
                step = 8;
            }
            else
            {
                if(dir == 2)
                    forwart_flag = 0;//后退消除标志
                step = 1;
            }
        }
        if(step<=0)
        {
            if(forwart_flag)
            {
                DBG_PRINT("IR_FOLLOW_GO",1);
                acc = 2;
                dir = 1;
                step = 8;
            }
            else
            {
                break;
            }

        }
        DBG_PRINT("IR_FOLLOW_MOVE >> ",dir);
        if(robot_irfollow_switch&0x04)
        {
            if(dir == 2)
            {
                robot_audio_voice_play(4);
                robot_action_cmd_play(3,20);//转一圈
                robot_irfollow_idle_time = 36;
                step = 0;
                break;
            }
            else
            {
                robot_action_cmd_play(dir,1);
                step--;
            }
        }
        else
            ROBOT_DELAY_MS(2000);
    }
    DBG_PRINT("IR_FOLLOW_MOVE # ",0);
    if(robot_irfollow_switch&0x04)
        robot_action_cmd_play(0,0);
    //DBG_PRINT("obs_touch = ",robot_irobs_touch_time);
    //DBG_PRINT("obs_break = ",robot_irobs_break_time);
}


#if IR_CAP_DEBUG
//#if 1
int g_cap_log[1024];
int g_cap_log_sz = 0;

void robot_irdebug_cap_init()
{
    g_cap_log_sz = 0;
}

void robot_irdebug_cap_add(uint16_t cap)
{
    if(g_cap_log_sz<1024)
    {
        g_cap_log[g_cap_log_sz++] = cap;
    }
}
void robot_irdebug_cap_show()
{
    int i;
    DBG_PRINT("cap sz = ",g_cap_log_sz);
    for(i=0;i<g_cap_log_sz;i++)
    {
        DBG_PRINT("cap -> ",g_cap_log[i]);
    }
}
#endif // 1

#define ROBOT_OBS_DET_IO IO_PORTA_04
#define ROBOT_OBS_SEND_IO IO_PORTC_07

volatile void *pwm_dev_obs = NULL;
volatile struct pwm_platform_data pwm_obs = {0};

void robot_irobs_send_init()
{
    struct pwm_platform_data pwm = {0};
    pwm_dev_obs = dev_open("pwm1", &pwm_obs);//打开PWM1设备
    if (!pwm_dev_obs) {
        DBG_PRINT("open pwm err !!!",0);
        return ;
    }
    robot_irobs_switch_set(0);
    DBG_PRINT("obs_pwm port",pwm_obs.port);
    DBG_PRINT("obs_pwm freq",pwm_obs.freq);
    DBG_PRINT("obs_pwm duty",(int)(pwm_obs.duty));

}

void robot_irobs_init()
{
    gpio_set_direction(ROBOT_OBS_DET_IO, 1);//鼻子
    gpio_set_pull_up(ROBOT_OBS_DET_IO, 0);
    gpio_set_pull_down(ROBOT_OBS_DET_IO, 1);
    gpio_set_die(ROBOT_OBS_DET_IO, 1);
    robot_irobs_send_init();
    STRDBG_PRINT("robot_irobs_init","done");
}

void robot_irobs_timer_once()//1ms
{
    if(!gpio_read(ROBOT_OBS_DET_IO))
    {
        robot_irobs_touch_time++;
        robot_irobs_break_time = 0;
    }
    else
    {
        robot_irobs_break_time++;
        if(robot_irobs_break_time>=10)
            robot_irobs_touch_time = 0;
    }
}

void robot_irobs_data_show()
{
    if(!robot_irobs_switch)
        return ;
    DBG_PRINT("obs_touch = ",robot_irobs_touch_time);
    DBG_PRINT("obs_break = ",robot_irobs_break_time);
}

void robot_irobs_switch_set(uint16_t set)
{
    robot_irobs_switch = set;
    if(set)
    {
        //DBG_PRINT("ROBOT_OBS_SEND_IO",1);
        if(pwm_dev_obs)
            dev_ioctl(pwm_dev_obs, PWM_RUN, (u32)&pwm_obs);//PWM开启
    }
    else
    {
        //DBG_PRINT("ROBOT_OBS_SEND_IO",0);
        if(pwm_dev_obs)
            dev_ioctl(pwm_dev_obs, PWM_STOP, (u32)&pwm_obs);//PWM停止
    }
}


uint16_t robot_irobs_test()
{
    int i;
    int j = 0;
    robot_irobs_switch_set(1);
    for(i=0;i<1000;i++)
    {
        if(!gpio_read(ROBOT_OBS_DET_IO))//touch
        {
            j++;
            STR_PRINT(".");
        }
        else
        {
            STR_PRINT("*");
        }
    }
    robot_irobs_switch_set(0);
    DBG_PRINT("IROBS GET >>",j);
    return (j>=10);
}

uint16_t robot_irobs_test_once()
{
    int i,j=0;
    for(i=0;i<8;i++)
    {
        if(!robot_irobs_test())
            j++;
        if(j>=2)
            break;
    }
    if(j<2)
        DBG_PRINT("IROBS_GET FAIL = ",j);
    else
        DBG_PRINT("IROBS_NULL TRY = ",i+1);
    return (j<3);
}

