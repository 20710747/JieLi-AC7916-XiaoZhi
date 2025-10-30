#include "robot_main.h"
#include "robot_debug.h"
#include "robot_bus.h"
#include "robot_motor.h"
#include "robot_adapter.h"

#define ROBOT_MOTOR_USE_TS15 1
#define ROBOT_MOTOR_USE_GPIO 2
#define ROBOT_MOTOR_USE_CONFIG ROBOT_MOTOR_USE_GPIO

//ts15
#if ROBOT_MOTOR_USE_CONFIG==ROBOT_MOTOR_USE_TS15
void robot_motor_set_ts15(int id,int speed)
{
    uint8_t para[4];
    uint8_t comm[20];
    uint16_t csz;
    int dir;

    if(id&0x01)
        dir = 0;
    else
        dir = 1;
    if(speed<0)
    {
        dir = 1-dir;
        speed = -speed;
    }
    DBG_PRINT("MOTOR_SET IDX =",id);
    DBG_PRINT("MOTOR_SET DIR =",dir);
    DBG_PRINT("MOTOR_SET SPD =",speed);

    para[0] = 0x00;
    para[1] = dir;
    para[2] = speed;
    csz = robot_bus_command_make(id,0x03,0x3f,3,para,comm);//motor_speed_set
    BUS_WRITE(comm,csz);
}

void robot_motor_init_ts15()
{
    int i;
    STRDBG_PRINT("MOTOR_INIT","TS15");
    for(i=1;i<=4;i++)
        robot_motor_set_ts15(i,0);
}

void robot_motor_exit_ts15()
{
    STRDBG_PRINT("MOTOR_EXIT","TS15");
}
#endif
//====

//gpio

//GPIO控制 一组GPIO控制一个电机 10前进 01后退 00惯性 11刹车
//u8 motor_move_flag;
#if ROBOT_MOTOR_USE_CONFIG==ROBOT_MOTOR_USE_GPIO
#define MOTOR_NUM (4)
#define MOTOR_SMOOTH_TIMER (10)

int smooth_flag = 0;
int smooth_update = 0;
int robot_motor_irq_handle_gpio = 0;

MOTOR_CTRL motor_ctrl[MOTOR_NUM+1]=
{
    {0, 0, 0,  0,  0,  0,    0},
    {1, IO_PORTA_01,    1,  0, 0,  0,  0},
    {2, IO_PORTA_03,    0,  0, 0,  0,  0},
    {3, IO_PORTA_05,    0,  0, 0,  0,  0},
    {4, IO_PORTA_07,    1,  0, 0,  0,  0},
    //{0, 0,  0,  0,  IO_PORTA_01,    0},
};

void robot_motor_move_gpio(u8 gpio,u8 mode)//控制一组电机，调节相邻两个IO口 mode = 1正转 mode = 0 反转
{
    DBG2_PRINT("MOTOR_MOVE >>",gpio-IO_PORTA_00,mode);
    gpio_direction_output(gpio,mode);
    gpio_direction_output(gpio + 1,1-mode);//相邻的IO口可以通过+1/-1操作
}

void robot_motor_stop_gpio(u8 gpio,u8 mode)//mode=1制动;=0惯性
{
    //DBG2_PRINT("MOTOR_STOP >>",gpio-IO_PORTA_00,mode);
    gpio_direction_output(gpio,mode);
    gpio_direction_output(gpio + 1,mode);
}

void robot_motor_speed_smooth(MOTOR_CTRL *ctrl)
{
    if(ctrl->target_speed==0xff)
        return;
    if(ctrl->target_speed > ctrl->current_speed)
        ctrl->current_speed++;
    if(ctrl->target_speed < ctrl->current_speed)
        ctrl->current_speed--;
}

void robot_motor_speed_get_dirlevel(MOTOR_CTRL *ctrl)
{
    if(ctrl->current_speed>=0)
    {
        ctrl->speed_dir = ctrl->type;
        ctrl->speed_level = ctrl->current_speed;
    }
    else
    {
        ctrl->speed_dir = 1 - ctrl->type;
        ctrl->speed_level = -ctrl->current_speed;
    }
}

void robot_motor_IRQ(void *para)//2ms执行一次
{
    static int motor_times = 0;
    static int motor_num = 0;
    int i;
    //平滑过渡 smooth
    //log_adc_add();//打印测试
    if(smooth_flag == -1)//四轮速度为0的情况
    {
        for(i=1;i<=MOTOR_NUM;i++)
        {
            motor_ctrl[i].current_speed = 0;
            motor_ctrl[i].speed_level = 0;
        }
        smooth_update = 0;
        smooth_flag = 0;
    }
    else if(smooth_flag >= (MOTOR_SMOOTH_TIMER-1))
    {
        if(smooth_update)//？
        {
            smooth_flag = 0;
            smooth_update = 0;

            for(i=1;i<=MOTOR_NUM;i++)
            {
                robot_motor_speed_smooth(&motor_ctrl[i]);
                robot_motor_speed_get_dirlevel(&motor_ctrl[i]);
                if(motor_ctrl[i].current_speed!=motor_ctrl[i].target_speed)
                    smooth_update = 1;
            }
        }
    }
    else
    {
        smooth_flag++;
    }
    motor_times++;
    if(motor_times > MOTOR_SMOOTH_TIMER)
    {
        motor_times = 1;

        for(i=1;i<=MOTOR_NUM;i++)
        {
            if(motor_ctrl[i].target_speed == 0xff)
            {
                robot_motor_stop_gpio(motor_ctrl[i].gpio,1);
            }
            else if(motor_ctrl[i].speed_level>=1)//有速度
            {
                robot_motor_move_gpio(motor_ctrl[i].gpio,motor_ctrl[i].speed_dir);
            }
        }
    }
    else
    {
        for(i=1;i<=MOTOR_NUM;i++)
        {
            if(motor_times > motor_ctrl[i].speed_level)
            {
                robot_motor_stop_gpio(motor_ctrl[i].gpio,0);
            }
        }
    }
}

void robot_motor_set_gpio(int id,int speed)
{
    motor_ctrl[id].target_speed = speed;
}

void rotob_motor_judge_smooth_flag()
{
    int i;
    for(i=1;i<=MOTOR_NUM;i++)
    {
        DBG2_PRINT("SPEED =",motor_ctrl[i].current_speed,motor_ctrl[i].target_speed);
    }
    for(i=1;i<=MOTOR_NUM;i++)
    {
        if(motor_ctrl[i].target_speed)
            break;
    }
    if(i>MOTOR_NUM)
    {
        smooth_flag = -1;
    }
    smooth_update = 1;
}

void robot_motor_init_gpio()
{
    int i;
    STRDBG_PRINT("MOTOR_INIT","GPIO");
    for(i=1;i<=MOTOR_NUM;i++)
    {
        motor_ctrl[i].target_speed = 0;
        motor_ctrl[i].current_speed = 0;
        gpio_direction_output(motor_ctrl[i].gpio,0);
        gpio_direction_output(motor_ctrl[i].gpio+1,0);
    }
    //
    if(robot_motor_irq_handle_gpio==0)
    {
        STRDBG_PRINT("MOTOR_IRQ","MAKE");
        robot_motor_irq_handle_gpio = sys_timer_add(0,robot_motor_IRQ,2);
    }
}

void robot_motor_exit_gpio()
{
    int i;
    STRDBG_PRINT("MOTOR_EXIT","GPIO");
    if(robot_motor_irq_handle_gpio)
    {
        STRDBG_PRINT("MOTOR_IRQ","DEL");
        sys_timer_del(robot_motor_irq_handle_gpio);
        robot_motor_irq_handle_gpio = 0;
    }
    for(i=1;i<=MOTOR_NUM;i++)
    {
        motor_ctrl[i].current_speed = 0;
        robot_motor_stop_gpio(motor_ctrl[i].gpio,0);
    }
}
#endif

//====

void robot_motor_init()
{
#if ROBOT_MOTOR_USE_CONFIG == ROBOT_MOTOR_USE_TS15
    robot_motor_init_ts15();
#else
    robot_motor_init_gpio();
#endif
}

void robot_motor_stop()
{
#if ROBOT_MOTOR_USE_CONFIG == ROBOT_MOTOR_USE_TS15
    int i;
    for(i=1;i<=MOTOR_NUM;i++)
        robot_motor_set_ts15(i,0);
#elif ROBOT_MOTOR_USE_CONFIG == ROBOT_MOTOR_USE_GPIO
    //robot_motor_exit_gpio();
    int i;
    for(i=1;i<=MOTOR_NUM;i++)
        robot_motor_set_gpio(i,0);
#endif // ROBOT_MOBOT_USE_CONFIG
}


uint16_t robot_motor_cmdGo(uint8_t *cmd)
{
    int i,j;
    int sum = 0;
    //做一个简单的奇偶校验就好了
    for(i=0;i<ROBOT_MOTOR_CMD_SZ;i++)
    {
        sum += cmd[i]&0x01;
    }
    if(sum&0x01)
        return 0;
    for(i=1;i<=4;i++)
    {
        j = cmd[i];
        if(j&0xF0)
            j = -(j&0x0F);
#if ROBOT_MOTOR_USE_CONFIG == ROBOT_MOTOR_USE_TS15
        robot_motor_set_ts15(i,j);
#elif ROBOT_MOTOR_USE_CONFIG == ROBOT_MOTOR_USE_GPIO
        robot_motor_set_gpio(i,j);
#endif
    }
#if ROBOT_MOTOR_USE_CONFIG == ROBOT_MOTOR_USE_GPIO
    rotob_motor_judge_smooth_flag();
#endif
    ROBOT_DELAY_MS(cmd[0]*5);
    if(cmd[0]==0)
        return 2;
    return 1;
}

void robot_motor_exit()
{
#if ROBOT_MOTOR_USE_CONFIG == ROBOT_MOTOR_USE_TS15
    robot_motor_exit_ts15();
#else
    robot_motor_exit_gpio();
#endif
}
