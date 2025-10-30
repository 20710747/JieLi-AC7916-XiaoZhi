
#include "robot_main.h"
#include "robot_debug.h"
#include "robot_bus.h"
#include "robot_head.h"
#include "robot_adapter.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

// ===================== head FSM =====================


// ================= 系统常量定义 =================


BlinkData blinkControl[BLINK_COUNT] =
{
    {BLINK_NONE},//无
    {BLINK_LEFT,//单眨眼左眼
    {1,BLINK_CLOSE_POS,BLINK_CLOSE_STEP_SINGLE,1,BLINK_CHEEK_PULL_POS,BLINK_CHEEK_STEP},
    {1,BLINK_OPEN_POS,BLINK_OPEN_STEP_SINGLE,1,BLINK_CHEEK_RECOVER_POS,BLINK_CHEEK_RECOVERY_STEP},
    {0,BLINK_ANTHOER_POS,BLINK_ANTOHER_STEP,0,0,0},
    {0,BLINK_OPEN_POS,BLINK_ANTOHER_STEP,0,0,0},
    BLINK_HOLD_FRAMES_SINGLE,BLINK_INTERVAL_FRAMES,BLINK_STOP_FRAMES},
    {BLINK_RIGHT,//单眨眼右眼
    {0,BLINK_ANTHOER_POS,BLINK_ANTOHER_STEP,0,0,0},
    {0,BLINK_OPEN_POS,BLINK_ANTOHER_STEP,0,0,0},
    {1,BLINK_CLOSE_POS,BLINK_CLOSE_STEP_SINGLE,1,BLINK_CHEEK_PULL_POS,BLINK_CHEEK_STEP},
    {1,BLINK_OPEN_POS,BLINK_OPEN_STEP_SINGLE,1,BLINK_CHEEK_RECOVER_POS,BLINK_CHEEK_RECOVERY_STEP},
    BLINK_HOLD_FRAMES_SINGLE,BLINK_INTERVAL_FRAMES,BLINK_STOP_FRAMES},
    {BLINK_BOTH,//双眨眼
    {1,BLINK_CLOSE_POS,BLINK_CLOSE_STEP_BOTH,0,0,0},
    {1,BLINK_OPEN_POS,BLINK_OPEN_STEP_BOTH,0,0,0},
    {1,BLINK_CLOSE_POS,BLINK_CLOSE_STEP_BOTH,0,0,0},
    {1,BLINK_OPEN_POS,BLINK_OPEN_STEP_BOTH,0,0,0},
    BLINK_HOLD_FRAMES_BOTH,BLINK_INTERVAL_FRAMES,BLINK_STOP_FRAMES},
    {BLINK_LEFT_SLOW,//慢眨眼左眼
    {1,BLINK_CLOSE_POS,BLINK_CLOSE_STEP_SLOW,1,BLINK_CHEEK_PULL_SLOW_POS,BLINK_CHEEK_STEP},
    {1,BLINK_OPEN_POS,BLINK_OPEN_STEP_SLOW,1,BLINK_CHEEK_RECOVER_POS,BLINK_CHEEK_RECOVERY_STEP},
    {0,BLINK_ANTHOER_POS,BLINK_ANTOHER_STEP,0,0,0},
    {0,BLINK_OPEN_POS,BLINK_ANTOHER_STEP,0,0,0},
    BLINK_HOLD_FRAMES_SLOW,BLINK_INTERVAL_FRAMES,BLINK_STOP_FRAMES},
    {BLINK_RIGHT_SLOW,//慢眨眼右眼
    {0,BLINK_ANTHOER_POS,BLINK_ANTOHER_STEP,0,0,0},
    {0,BLINK_OPEN_POS,BLINK_ANTOHER_STEP,0,0,0},
    {1,BLINK_CLOSE_POS,BLINK_CLOSE_STEP_SLOW,1,BLINK_CHEEK_PULL_SLOW_POS,BLINK_CHEEK_STEP},
    {1,BLINK_OPEN_POS,BLINK_OPEN_STEP_SLOW,1,BLINK_CHEEK_RECOVER_POS,BLINK_CHEEK_RECOVERY_STEP},
    BLINK_HOLD_FRAMES_SLOW,BLINK_INTERVAL_FRAMES,BLINK_STOP_FRAMES},
};


// 全局控制器实例
static int head_init_flag = 0;

static HeadController head;


// ================= 随机数生成 =================
// 简单整数随机数生成器
static int16_t rand_range(int16_t min, int16_t max) {
    //int a = (rand() % (max - min + 1));
    //int b = min;
    //extern int servo_target_debug;
    //if(servo_target_debug)
    //    DBG2_PRINT("rand_range = ",a,b);
    //return a + b;
    if(min==max)
        return min;
    if(min>max)
    {
        int temp = max;
        max = min;
        min = temp;
    }
    return min + (rand() % (max - min + 1));
}

// 概率判断(千分比)
static uint8_t probability_check(uint16_t prob) {
    return (rand() % 1000) < prob;
}

// ================= 舵机控制核心函数 =================
// 初始化舵机
static void servo_init(Servo* s, int16_t init_pos, uint16_t step) {
    s->current = init_pos;
    s->target = init_pos;
    s->step = step;
}

void servo_read(int idx,Servo* s)
{
    const char pos_name[NUM_SERVOS][12] =
    {
        "neck","mouth","eyes",
        "l_eyelid","r_eyelid",
        "l_cheek","r_cheek",
        "tongue","spare",
    };
    DBG_PRINTF("READ [%d][%s]",idx+1,pos_name[idx]);
    DBG_PRINTF("current = %d target = %d",s->current,s->target);
    DBG_PRINTF("current = %d step = %d send = %d",s->current,s->step,s->send);
}

void servo_read_all()
{
    for (int i = 0; i < NUM_SERVOS; i++)
    {
        servo_read(i,&head.servos[i]);
    }
}

// 更新舵机位置
static void servo_update(Servo* s) {
    int16_t diff = s->target - s->current;
    int16_t move = 0;

    if (diff != 0) {
        int16_t max_step = s->step + rand_range(-(s->step/8), s->step/8); // ±15%变化
        move = (abs(diff) > max_step) ?
               (diff > 0 ? max_step : -max_step) : diff;
        s->send = s->current + move*SERVO_AMPLIFICATION_FACTOR_MOVE;
        s->current += move;
        //if(head.state.speak_state != SPEAK_NONE)
        //    DBG2_PRINT("move = ",move,s->step);
    }
    else
    {
        s->send = s->current;
        //if(head.state.speak_state != SPEAK_NONE)
        //    DBG2_PRINT("move = ",0,s->step);
    }
}

static servo_show(int idx)
{
    Servo* s = &head.servos[idx];
    DBG_PRINTF("<%d>:[%d -> %d] send = %d",idx,s->current,s->target,s->send);
}
int servo_target_debug = 0;
// 设置舵机目标和步长
static void set_servo_target_step_range(Servo* s, int16_t target, uint16_t step,int16_t range) {

    int16_t variation = rand_range(-range, range); // ±15%变化
    s->target = target + variation;
    if(servo_target_debug)
    {
        DBG_PRINT("variation = ",variation);
        DBG2_PRINT("target = ",target,s->target);
    }
    if (s->target < SERVO_MIN_POS) s->target = SERVO_MIN_POS;
    if (s->target > SERVO_MAX_POS) s->target = SERVO_MAX_POS;
    s->step = step;
}

static void set_servo_target_step_norange(Servo* s, int16_t target, uint16_t step)
{
    set_servo_target_step_range(s,target,step,0);
}

static void set_servo_target_step(Servo* s, int16_t target, uint16_t step)
{
    int16_t range = abs(target)/8;
    if(range > RANGE_MAX) range = RANGE_MAX;
    set_servo_target_step_range(s,target,step,range);
}



static void set_servo_step(Servo* s,uint16_t step) {
    s->step = step;
}


static void adjust_servo_target_step(Servo* s, int16_t delta_target, uint16_t delta_step)
{
    s->target += delta_target;
    s->step += delta_step;
    if (s->target < SERVO_MIN_POS) s->target = SERVO_MIN_POS;
    if (s->target > SERVO_MAX_POS) s->target = SERVO_MAX_POS;
    if(s->step<0)
        s->step = 0;

}

// ================= 行为模块 =================
// 初始化头部控制器
void head_init(void) {
    srand(time(NULL)); // 初始化随机种子

    // 初始化所有舵机
    servo_init(&head.servos[SERVO_NECK], 0, DEFAULT_STEP);
    servo_init(&head.servos[SERVO_MOUTH], 0, DEFAULT_STEP);
    servo_init(&head.servos[SERVO_EYES], 0, DEFAULT_STEP);
    servo_init(&head.servos[SERVO_L_EYELID], 0, DEFAULT_STEP);
    servo_init(&head.servos[SERVO_R_EYELID], 0, DEFAULT_STEP);
    servo_init(&head.servos[SERVO_L_CHEEK], 0, DEFAULT_STEP);
    servo_init(&head.servos[SERVO_R_CHEEK], 0, DEFAULT_STEP);
    servo_init(&head.servos[SERVO_TONGUE], 0, DEFAULT_STEP);
    servo_init(&head.servos[SERVO_SPARE], 0, DEFAULT_STEP);

    // 初始化状态
    head.state.last_blink_time = 0;
    head.state.next_blink_interval = rand_range(BLINK_MIN_INTERVAL, BLINK_MAX_INTERVAL);
    head.state.last_smile_time = 0;
    head.state.next_smile_interval = rand_range(SMILE_MIN_INTERVAL, SMILE_MAX_INTERVAL);
    head.state.breath_counter = 0;
    head.state.neck_timer = NECK_MIN_MOVE_FRAMES;
    head.state.flag_auto_blink = 0;
    head.state.flag_auto_smile = 0;

    head.state.blink_type = BLINK_NONE;
    head.state.blink_count = 0;
    head.state.blink_state = 0;

    head.state.smile_type = SMILE_NONE;
    head.state.smile_state = 0;

    head.state.speak_state = SPEAK_NONE;
    head.state.lick_state = LICK_NONE;
    head.state.rub_state = RUB_NONE;

    head.state.neck_state = NECK_NONE;
    head.state.frame_count = 0;
    head.state.neck_dir = 1;

    head.msg.flag = 0;
}

//处理消息



void head_debug_set(int data1,int data2)
{
    switch(data1)
    {
    case MSG_DEBUG_NECKTIME:
        TS15_IDTimeSend(1,data2);
        break;
    case MSG_DEBUG_NECTSET:
        if(data2)
            handle_neck_start();
        else
            handle_neck_stop();
        break;
    }
}

static void handle_msg(void)
{
    if(head.msg.flag==2)
    {
        head.msg.flag = 1;
        switch(head.msg.type)
        {
        case MSG_BLINK:
            head_blink(head.msg.data[0],head.msg.data[1]);
            break;
        case MSG_SMILE:
            head_smile(head.msg.data[0]);
            break;
        case MSG_SPEAK:
            //void head_speak(SpeakState state, uint16_t speed, uint16_t amplitude)
            head_speak(head.msg.data[0],head.msg.data[1],head.msg.data[2]);
            break;
        case MSG_LICK:
            head_lick(head.msg.data[0],head.msg.data[1]);
            break;
        case MSG_RUB:
            head_rub(head.msg.data[0],head.msg.data[1],head.msg.data[2]);
            break;
        case MSG_SET:
            head_flag_set(head.msg.data[0],head.msg.data[1]);
            break;
        case MSG_DEBUG:
            head_debug_set(head.msg.data[0],head.msg.data[1]);
            break;
        }
        head.msg.flag = 0;
    }
}


// 处理待机行为
static void handle_idle() {

    //static int keyuseflag = 0;
    // 眼球随机运动
    if (abs(head.servos[SERVO_EYES].target-head.servos[SERVO_EYES].current)<=EYE_STEP_MAX)
    {
        static int static_count = 0;
        if (probability_check(EYE_MOVE_PROB2) || (static_count>=24)) {
            int16_t eye_target = rand_range(-EYE_RANGE, EYE_RANGE);
            set_servo_target_step(&head.servos[SERVO_EYES], eye_target, rand_range(1,EYE_STEP_MAX));
            static_count = 0;
            //eyedebug = 1;
        }
        else
        {
            //eyedebug = 2;
            static_count++;
        }
    }
    else
    {
        if (probability_check(EYE_MOVE_PROB)) {
            int16_t eye_target = rand_range(-EYE_RANGE, EYE_RANGE);
            set_servo_target_step(&head.servos[SERVO_EYES], eye_target, rand_range(1,EYE_STEP_MAX));
            //eyedebug = 3;
        }
        else
        {
            //eyedebug = 4;
        }
    }
    //DBG_PRINT("EYE_DEBUG = ",eyedebug);
    //DBG2_PRINT("EYE_INFO = ",head.servos[SERVO_EYES].current,head.servos[SERVO_EYES].target);


    switch(head.state.neck_state)
    {
        case NECK_NONE:
            break;
        case NECK_IDLE:
            head.state.neck_state = NECK_RUN;
            if(head.state.neck_dir==1)
            {
                head.state.neck_target = rand_range(
                                            MAX(-NECK_RANGE_HALF,head.state.neck_target-NECK_RANGE_MAX),
                                            MIN(0,head.state.neck_target-NECK_RANGE_MIN));
            }
            else
            {
                head.state.neck_target = rand_range(
                                            MAX(0,head.state.neck_target+NECK_RANGE_MIN),
                                            MIN(NECK_RANGE_HALF,head.state.neck_target+NECK_RANGE_MAX));
            }
            head.state.neck_step = rand_range(3,5);
            head.state.neck_timer = rand_range(18,24);
            set_servo_target_step(&head.servos[SERVO_NECK],head.state.neck_target,head.state.neck_step);
            break;
        case NECK_SPEAK:
            head.state.neck_state = NECK_RUN;
            break;
        case NECK_LICK:
            head.state.neck_state = NECK_RUN;
            break;
        case NECK_RUN:
            head.state.neck_timer--;
            if(head.state.neck_timer<0)
            {
                head.state.neck_state = NECK_NONE;
                head.state.neck_dir = 3 - head.state.neck_dir;
            }

            else
            {
                if(abs(head.state.neck_target-head.servos[SERVO_NECK].current)<NECK_RANGE_MAX)
                {
                    int delta_step = (head.state.neck_step *(NECK_RANGE_MAX-abs(head.state.neck_target-head.servos[SERVO_NECK].current)))/
                                        (2*NECK_RANGE_MAX);
                    set_servo_step(&head.servos[SERVO_NECK],head.state.neck_step - delta_step);
                }
            }

            //DBG2_PRINT("neck_go servo",head.servos[SERVO_NECK].target,head.servos[SERVO_NECK].current);
            //DBG2_PRINT("neck_go time&step = ",head.state.neck_timer,head.servos[SERVO_NECK].step);
            break;
    }
    if(head.state.neck_state)
        DBG_PRINTF("neck = %d %d %d",head.state.neck_state,head.servos[SERVO_NECK].current,head.servos[SERVO_NECK].target);
    if(head.state.speak_state)
    {

    }
    else
    {
        // 呼吸模拟
        #if 1
        float breath_phase = (head.state.breath_counter % BREATH_CYCLE_FRAMES) * 2 * M_PI / BREATH_CYCLE_FRAMES;
        int16_t breath_val = abs(sin(breath_phase) * BREATH_MOUTH_RANGE);
        int16_t breath_cheek = abs(breath_val) * BREATH_CHEEK_RANGE / BREATH_MOUTH_RANGE;
        int16_t breath_lid = abs(breath_val) * BREATH_LID_RANGE / BREATH_MOUTH_RANGE;
        //DBG_PRINTF("breath_val = %d %d %d",breath_val,breath_cheek,breath_lid + BREATH_LID_BASE);
        //set_servo_target(&head.servos[SERVO_MOUTH], breath_val);//取消呼吸的时候嘴巴会动，仅仅
        set_servo_target_step_norange(&head.servos[SERVO_L_CHEEK], breath_cheek,BREATH_CHEEK_STEP_MAX);
        set_servo_target_step_norange(&head.servos[SERVO_R_CHEEK], breath_cheek,BREATH_CHEEK_STEP_MAX);


        //int16_t base_eyelid = rand_range(-EYELID_RANGE/2, EYELID_RANGE/8);
        int16_t base_eyelid = breath_lid + BREATH_LID_BASE;
        int16_t diff = rand_range(-EYELID_DIFF_MAX, EYELID_DIFF_MAX);
        set_servo_target_step(&head.servos[SERVO_L_EYELID], base_eyelid + diff/2, EYELID_STEP_MAX);
        set_servo_target_step(&head.servos[SERVO_R_EYELID], base_eyelid - diff/2, EYELID_STEP_MAX);

        head.state.breath_counter++;
        #else
        robot_key_buskey_read();
        #endif

        //DBG2_PRINT("breath_val = ",breath_val,abs(breath_val) * BREATH_CHEEK_RANGE / BREATH_MOUTH_RANGE);
    }
    #if 1
    // 随机眨眼
    if (head.state.flag_auto_blink)
    {
        if (head.state.frame_count - head.state.last_blink_time > head.state.next_blink_interval) {
            if (probability_check(DOUBLE_BLINK_PROB)) {
                head_blink(BLINK_BOTH, 2);
            } else {
                head_blink(BLINK_BOTH, 1);
            }
        }
    }

    // 随机微笑
    if(head.state.flag_auto_smile)
    {
        if (head.state.frame_count - head.state.last_smile_time > head.state.next_smile_interval) {

            head_smile(SMILE_SMALL);
            head.state.last_smile_time = head.state.frame_count;
            head.state.next_smile_interval = rand_range(SMILE_MIN_INTERVAL, SMILE_MAX_INTERVAL);
        }
    }
    #endif
}


// 处理眨眼行为
static void handle_blink(void) {
    int left_done,right_done;
    int type = head.state.blink_type;
    if (type == BLINK_NONE) return;
    //DBG_PRINT("head.state.blink_state = ",head.state.blink_state);
    switch (head.state.blink_state) {
        case 0: // 开始眨眼
            //head.state.blink_timer = rand_range(4, 5); // 闭眼时间
            head.state.blink_state = 1;
            break;

        case 1: // 闭眼过程
            set_servo_target_step_norange(&head.servos[SERVO_L_EYELID],
                    blinkControl[type].close_left.lid_pos, blinkControl[type].close_left.lid_step);
            set_servo_target_step_norange(&head.servos[SERVO_R_EYELID],
                    blinkControl[type].close_right.lid_pos, blinkControl[type].close_right.lid_step);
            if(blinkControl[type].close_left.cheek_set)
            {
                set_servo_target_step(&head.servos[SERVO_L_CHEEK],
                    blinkControl[type].close_left.cheek_pos, blinkControl[type].close_left.cheek_step);
            }
            if(blinkControl[type].close_right.cheek_set)
            {
                set_servo_target_step(&head.servos[SERVO_R_CHEEK],
                    blinkControl[type].close_right.cheek_pos, blinkControl[type].close_right.cheek_step);
            }
            // 设置眼皮目标
            #if 0
            if ((head.state.blink_type == BLINK_LEFT)||(head.state.blink_type == BLINK_LEFT_HOLD)) {
                set_servo_target_step_norange(&head.servos[SERVO_L_EYELID], BLINK_CLOSE_POS, BLINK_CLOSE_STEP_SINGLE);
                set_servo_target_step_norange(&head.servos[SERVO_R_EYELID], BLINK_OPEN_POS, BLINK_CLOSE_STEP_SINGLE);
                set_servo_target_step(&head.servos[SERVO_L_CHEEK], BLINK_CHEEK_PULL, BLINK_CHEEK_STEP);
                set_servo_target_step(&head.servos[SERVO_R_CHEEK], 0, BLINK_CHEEK_STEP_ANTHOER);
            }
            if ((head.state.blink_type == BLINK_RIGHT)||(head.state.blink_type == BLINK_RIGHT_HOLD)) {
                set_servo_target_step_norange(&head.servos[SERVO_R_EYELID], BLINK_CLOSE_POS, BLINK_CLOSE_STEP_SINGLE);
                set_servo_target_step_norange(&head.servos[SERVO_L_EYELID], BLINK_OPEN_POS, BLINK_CLOSE_STEP_SINGLE);
                set_servo_target_step(&head.servos[SERVO_R_CHEEK], BLINK_CHEEK_PULL, BLINK_CHEEK_STEP);
                set_servo_target_step(&head.servos[SERVO_L_CHEEK], 0, BLINK_CHEEK_STEP_ANTHOER);
            }
            if (head.state.blink_type == BLINK_BOTH) {
                set_servo_target_step_norange(&head.servos[SERVO_L_EYELID], BLINK_CLOSE_POS, BLINK_CLOSE_STEP_BOTH);
                set_servo_target_step_norange(&head.servos[SERVO_R_EYELID], BLINK_CLOSE_POS, BLINK_CLOSE_STEP_BOTH);
            }
            #endif
            // 检查是否完成闭眼
            left_done = (blinkControl[type].close_left.lid_check==0) ||
                            (head.servos[SERVO_L_EYELID].current >= blinkControl[type].close_left.lid_pos-6);
            right_done = (blinkControl[type].close_right.lid_check==0) ||
                             (head.servos[SERVO_R_EYELID].current >= blinkControl[type].close_right.lid_pos-6);
            //DBG2_PRINT("done1 = ",left_done,right_done);
            if (left_done && right_done) {
                head.state.blink_state = 2;
                head.state.blink_timer = blinkControl[type].hold_frame;
            }

            break;

        case 2: // 闭眼保持
            if (--head.state.blink_timer == 0) {
                head.state.blink_state = 3; // 进入睁眼过程
            }
            break;

        case 3: // 睁眼过程
            // 恢复眼皮位置
            set_servo_target_step_norange(&head.servos[SERVO_L_EYELID],
                    blinkControl[type].open_left.lid_pos, blinkControl[type].open_left.lid_step);
            set_servo_target_step_norange(&head.servos[SERVO_R_EYELID],
                    blinkControl[type].open_right.lid_pos, blinkControl[type].open_right.lid_step);
            if(blinkControl[type].open_left.cheek_set)
            {
                set_servo_target_step(&head.servos[SERVO_L_CHEEK],
                    blinkControl[type].open_left.cheek_pos, blinkControl[type].open_left.cheek_step);
            }
            if(blinkControl[type].open_right.cheek_set)
            {
                set_servo_target_step(&head.servos[SERVO_R_CHEEK],
                    blinkControl[type].open_right.cheek_pos, blinkControl[type].open_right.cheek_step);
            }
            #if 0
            if (head.state.blink_type == BLINK_LEFT || head.state.blink_type == BLINK_BOTH) {
                if(head.state.blink_type == BLINK_BOTH)
                    set_servo_target_step_norange(&head.servos[SERVO_L_EYELID], BLINK_OPEN_POS, BLINK_OPEN_STEP_BOTH);
                else
                    set_servo_target_step_norange(&head.servos[SERVO_L_EYELID], BLINK_OPEN_POS, BLINK_OPEN_STEP_SINGLE);
                //set_servo_target_step(&head.servos[SERVO_L_CHEEK], 0, BLINK_CHEEK_STEP_RECOVER);
            }
            if (head.state.blink_type == BLINK_RIGHT || head.state.blink_type == BLINK_BOTH) {
                if(head.state.blink_type == BLINK_BOTH)
                    set_servo_target_step_norange(&head.servos[SERVO_R_EYELID], BLINK_OPEN_POS, BLINK_OPEN_STEP_BOTH);
                else
                    set_servo_target_step_norange(&head.servos[SERVO_R_EYELID], BLINK_OPEN_POS, BLINK_OPEN_STEP_SINGLE);
               // set_servo_target_step(&head.servos[SERVO_R_CHEEK], 0, BLINK_CHEEK_STEP_RECOVER);
            }
            if(head.state.blink_type != BLINK_BOTH)
            {
                set_servo_target_step_norange(&head.servos[SERVO_L_CHEEK], BLINK_OPEN_POS, BLINK_CHEEK_RECOVERY_STEP);
                set_servo_target_step_norange(&head.servos[SERVO_R_CHEEK], BLINK_OPEN_POS, BLINK_CHEEK_RECOVERY_STEP);
            }
#if 0
            // 单眨眼时恢复脸颊位置
            if (head.state.blink_type == BLINK_LEFT) {
                set_servo_target_step(&head.servos[SERVO_L_CHEEK], 0, BLINK_CHEEK_RECOVERY_STEP);
            } else if (head.state.blink_type == BLINK_RIGHT) {
                set_servo_target_step(&head.servos[SERVO_R_CHEEK], 0, BLINK_CHEEK_RECOVERY_STEP);
            }
#endif
            #endif
            // 检查是否完成睁眼
            left_done = (blinkControl[type].open_left.lid_check==0) ||
                            (head.servos[SERVO_L_EYELID].current <= blinkControl[type].open_left.lid_pos+6);
            right_done = (blinkControl[type].open_right.lid_check==0) ||
                             (head.servos[SERVO_R_EYELID].current <= blinkControl[type].open_right.lid_pos+6);

            if (left_done && right_done) {
                head.state.blink_count--;
                if (head.state.blink_count > 0) {
                    head.state.blink_state = 4; // 准备下一次眨眼
                    head.state.blink_timer = blinkControl[type].interval_frame;
                } else {
                    head.state.blink_state = 5; // 准备结束
                    head.state.blink_timer = blinkControl[type].stop_frame;
                }
            }

            break;

        case 4: // 眨眼间隔
            if (--head.state.blink_timer <= 0) {
                head.state.blink_state = 0; // 开始下一次眨眼
            }
            break;
        case 5: // 眨眼结束
            if (--head.state.blink_timer <= 0) {
                head.state.blink_state = 0;
                head.state.blink_type = BLINK_NONE; // 结束眨眼
            }
            break;
    }
    DBG_PRINTF("blink left_lid = %d %d %d",head.servos[SERVO_L_EYELID].current,head.servos[SERVO_L_EYELID].target,head.servos[SERVO_L_EYELID].step);
    DBG_PRINTF("blink right_lid = %d %d %d",head.servos[SERVO_R_EYELID].current,head.servos[SERVO_R_EYELID].target,head.servos[SERVO_R_EYELID].step);
    DBG_PRINTF("blink left_cheek = %d %d %d",head.servos[SERVO_L_CHEEK].current,head.servos[SERVO_L_CHEEK].target,head.servos[SERVO_L_CHEEK].step);
    DBG_PRINTF("blink right_cheek = %d %d %d",head.servos[SERVO_R_CHEEK].current,head.servos[SERVO_R_CHEEK].target,head.servos[SERVO_R_CHEEK].step);
}

// 处理微笑行为
static void handle_smile(void) {
    if (head.state.smile_type == SMILE_NONE) return;
    //DBG_PRINT("head.state.smile_state = ",head.state.smile_state);
    switch (head.state.smile_state) {
        case 0: // 张嘴阶段
            set_servo_target_step(&head.servos[SERVO_MOUTH], SMILE_MOUTH_OPEN, SMILE_MOUTH_STEP);
            set_servo_target_step(&head.servos[SERVO_L_CHEEK], 0, SMILE_RECOVERY_STEP);
            set_servo_target_step(&head.servos[SERVO_R_CHEEK], 0, SMILE_RECOVERY_STEP);
            if ((head.servos[SERVO_MOUTH].current >= SMILE_MOUTH_OPEN - 5)||(head.state.smile_type != SMILE_BIG)) {
                head.state.smile_state = 1;
            }
            break;
        case 1: // 微笑阶段
            if ((head.state.smile_type == SMILE_BIG)||(head.state.smile_type == SMILE_AFTERSPEAK)) {
                set_servo_target_step(&head.servos[SERVO_L_CHEEK], SMILE_BIG_CHEEK, SMILE_CHEEK_STEP);
                set_servo_target_step(&head.servos[SERVO_R_CHEEK], SMILE_BIG_CHEEK, SMILE_CHEEK_STEP);
                set_servo_target_step(&head.servos[SERVO_L_EYELID], SMILE_EYE_CLOSE, SMILE_EYE_STEP);
                set_servo_target_step(&head.servos[SERVO_R_EYELID], SMILE_EYE_CLOSE, SMILE_EYE_STEP);
				set_servo_target_step(&head.servos[SERVO_MOUTH], SMILE_MOUTH_CLOSE, SMILE_MOUTH_CLOSE_STEP);

            } else {
				set_servo_target_step(&head.servos[SERVO_L_CHEEK], SMILE_SMALL_CHEEK, SMILE_CHEEK_STEP);
                set_servo_target_step(&head.servos[SERVO_R_CHEEK], SMILE_SMALL_CHEEK, SMILE_CHEEK_STEP);
                set_servo_target_step(&head.servos[SERVO_MOUTH], SMILE_MOUTH_CLOSE, SMILE_MOUTH_CLOSE_STEP);
            }

            // 检查是否到位
            //DBG2_PRINT("LEFT >> ",head.servos[SERVO_L_CHEEK].current,head.servos[SERVO_L_CHEEK].target);
            //DBG2_PRINT("RIGHT >> ",head.servos[SERVO_R_CHEEK].current,head.servos[SERVO_R_CHEEK].target);
            if (abs(head.servos[SERVO_L_CHEEK].current - head.servos[SERVO_L_CHEEK].target) <= 8 &&
                abs(head.servos[SERVO_R_CHEEK].current - head.servos[SERVO_R_CHEEK].target) <= 8) {
                head.state.smile_timer = (head.state.smile_type != SMILE_SMALL)?(SMILE_BIG_HOLD_FRAMES):(SMILE_SMALL_HOLD_FRAMES);
                head.state.smile_state = 2;
            }
            break;

        case 2: // 保持微笑
            //DBG_PRINT("smile hold >>",head.state.smile_timer);
            if (--head.state.smile_timer <= 0) {

                head.state.smile_state = 3;
            }
            break;

        case 3: // 恢复阶段
            set_servo_target_step(&head.servos[SERVO_L_CHEEK], 0, SMILE_RECOVERY_STEP);
            set_servo_target_step(&head.servos[SERVO_R_CHEEK], 0, SMILE_RECOVERY_STEP);
            set_servo_target_step(&head.servos[SERVO_L_EYELID], 0, SMILE_EYE_STEP);
            set_servo_target_step(&head.servos[SERVO_R_EYELID], 0, SMILE_EYE_STEP);

            // 检查是否恢复完成
            if (head.servos[SERVO_L_CHEEK].current <= 8 &&
                head.servos[SERVO_R_CHEEK].current <= 8 ) {
                head.state.smile_type = SMILE_NONE;
            }
            break;
    }
}

int get_speak_status()
{
    switch(head.state.speak_state)
    {
    case SPEAK_OPEN:
        return 1;
    case SPEAK_CLOSE:
        return 2;
    case SPEAK_OPEN_RUN:
        if(head.servos[SERVO_MOUTH].current < head.servos[SERVO_MOUTH].target - 12)
            return 3;//运行中。
        else
            return 4;//到达
    case SPEAK_CLOSE_RUN:
        if(head.servos[SERVO_MOUTH].current > head.servos[SERVO_MOUTH].target + 10)
            return 5;//运行中
        else
            return 6;//到达
    }
    return 0;
}
// 处理讲话行为
static void handle_speak(void) {
    int delta_step;
    static int speakf = 0;
    if(head.state.speak_state == SPEAK_NONE)
    {
        if(speakf)
        {
            DBG_PRINT("speak use frame = ",speakf);
            speakf = 0;
        }
        return;
    }
    //DBG2_PRINT("handle_speak = ",head.state.speak_state,speakf);
    speakf++;
    // 根据状态设置目标
    switch (head.state.speak_state) {
        case SPEAK_OPEN:
            set_servo_target_step(&head.servos[SERVO_MOUTH],
                                 head.state.speak_amplitude,
                                 head.state.speak_speed);
            //set_servo_target_step(&head.servos[SERVO_TONGUE],
            //                     rand_range(SPEAK_TONGUE_MIN, SPEAK_TONGUE_MAX),
            //                     SPEAK_TONGUE_STEP);
            set_servo_target_step(&head.servos[SERVO_L_CHEEK],
                                SPEAK_CHEEK_OPEN,SPEAK_CHEEK_STEP);
            set_servo_target_step(&head.servos[SERVO_R_CHEEK],
                                SPEAK_CHEEK_OPEN,SPEAK_CHEEK_STEP);

            set_servo_target_step(&head.servos[SERVO_L_EYELID],
                                rand_range(SPEAK_EYELID_OPEN_MIN,SPEAK_EYELID_OPEN_MAX),SPEAK_EYELID_STEP);
            set_servo_target_step(&head.servos[SERVO_R_EYELID],
                                rand_range(SPEAK_EYELID_OPEN_MIN,SPEAK_EYELID_OPEN_MAX),SPEAK_EYELID_STEP);

            head.state.speak_state = SPEAK_OPEN_RUN;
            break;
        case SPEAK_OPEN_RUN:
            delta_step = (3 * head.state.speak_speed * head.servos[SERVO_MOUTH].current)/(5 * head.state.speak_amplitude);
            //if(get_speak_status()==3)
            //    DBG2_PRINT("delta_step = ",delta_step,head.state.speak_speed-delta_step);
            set_servo_step(&head.servos[SERVO_MOUTH],head.state.speak_speed-delta_step);
            break;
        case SPEAK_CLOSE:
            set_servo_target_step(&head.servos[SERVO_MOUTH], head.state.speak_amplitude, head.state.speak_speed);
            //set_servo_target_step(&head.servos[SERVO_TONGUE], 0, SPEAK_TONGUE_STEP);
            set_servo_target_step(&head.servos[SERVO_L_CHEEK],
                                SPEAK_CHEEK_CLOSE,SPEAK_CHEEK_STEP);
            set_servo_target_step(&head.servos[SERVO_R_CHEEK],
                                SPEAK_CHEEK_CLOSE,SPEAK_CHEEK_STEP);

            set_servo_target_step(&head.servos[SERVO_L_EYELID],
                                rand_range(SPEAK_EYELID_CLOSE_MIN,SPEAK_EYELID_CLOSE_MAX),SPEAK_EYELID_STEP);
            set_servo_target_step(&head.servos[SERVO_R_EYELID],
                                rand_range(SPEAK_EYELID_CLOSE_MIN,SPEAK_EYELID_CLOSE_MAX),SPEAK_EYELID_STEP);

            head.state.speak_state = SPEAK_CLOSE_RUN;
            break;
        case SPEAK_CLOSE_RUN:
            delta_step = (head.servos[SERVO_MOUTH].current>=SPEAK_MOUTH_FULL)?(0):
                ((3 * head.state.speak_speed * (SPEAK_MOUTH_FULL-head.servos[SERVO_MOUTH].current))/(5 * (SPEAK_MOUTH_FULL+10)));
            //if(get_speak_status()==5)
            //    DBG2_PRINT("delta_step = ",delta_step,head.state.speak_speed-delta_step);
            set_servo_step(&head.servos[SERVO_MOUTH],head.state.speak_speed-delta_step);
            break;

    }
    //if((get_speak_status()!=4)&&(get_speak_status()!=6))
    //    DBG2_PRINT("handle_speak servo = ",head.servos[SERVO_MOUTH].current,head.servos[SERVO_MOUTH].target);
    // 脖子配合运动
    //static int8_t neck_dir = 1;
    //if (head.state.speak_state == SPEAK_OPEN) {
    //    set_servo_target_step(&head.servos[SERVO_NECK],
    //                         neck_dir * SPEAK_NECK_RANGE/2,
    //                         SPEAK_NECK_STEP);
    //    neck_dir = -neck_dir;
    //}
}

int get_lick_open(int mode)
{
    switch(mode)
    {
    case 1:
        return LICK_MOUTH_OPEN_MODE1;
    case 2:
        return LICK_MOUTH_OPEN_MODE2;
    case 3:
        return LICK_MOUTH_OPEN_MODE3;
    case 4:
        return LICK_MOUTH_OPEN_MODE4;
    default:
        return LICK_MOUTH_OPEN_MODE4;
    }
}
// 处理吐舌头行为
static void handle_lick(void) {
    if (head.state.lick_state == LICK_NONE) return;

    switch (head.state.lick_state) {
        case LICK_OUT: // 伸出舌头
            // 先张嘴
            int lick_mouth_open = get_lick_open(head.state.lick_mode);
            if (head.servos[SERVO_MOUTH].current < lick_mouth_open-8) {
                set_servo_target_step(&head.servos[SERVO_MOUTH], lick_mouth_open, LICK_MOUTH_STEP);
            }
            // 然后伸舌头
            else if (head.servos[SERVO_TONGUE].current < LICK_TONGUE_OUT) {
                set_servo_target_step(&head.servos[SERVO_TONGUE], LICK_TONGUE_OUT, LICK_TONGUE_STEP);
                set_servo_target_step(&head.servos[SERVO_MOUTH], LICK_MOUTH_FULL, LICK_MOUTH_STEP);
            }
            // 完全伸出
            else {
                head.state.lick_timer = LICK_HOLD_FRAMES;
                head.state.lick_state = LICK_HOLD;
            }
            DBG2_PRINT("lick move",head.servos[SERVO_MOUTH].current,head.servos[SERVO_TONGUE].current);
            break;

        case LICK_HOLD: // 保持伸出状态
            // 脖子晃动
            //if (head.state.neck_move) {
            //    set_servo_target_step(&head.servos[SERVO_NECK],
            //                        rand_range(-LICK_NECK_RANGE/2, LICK_NECK_RANGE/2),
            //                        LICK_NECK_STEP);
            //}

            if (--head.state.lick_timer == 0) {
                head.state.lick_state = LICK_IN;
            }
            break;

        case LICK_IN: // 收回舌头
            // 先收舌头
            if (head.servos[SERVO_TONGUE].current > 0) {
                set_servo_target_step(&head.servos[SERVO_TONGUE], 0, LICK_TONGUE_RETRACT_STEP);
            }
            // 然后闭嘴
            else if (head.servos[SERVO_MOUTH].current > 0) {
                set_servo_target_step(&head.servos[SERVO_MOUTH], 0, LICK_MOUTH_STEP);
            }
            // 完全收回
            else {
                head.state.lick_state = LICK_NONE;
            }
            break;
    }
}

// 处理蹭行为
static void handle_rub(void) {
    if (head.state.rub_state == RUB_NONE) return;

    int8_t dir = head.state.rub_dir;
    //DBG2_PRINT("handle_rub state = ",head.state.rub_state,dir);
    //DBG2_PRINT("rub_neck = ",head.servos[SERVO_NECK].current,head.servos[SERVO_NECK].target);
    switch (head.state.rub_state) {
        case RUB_MOVE1: // 第一步移动
            servo_target_debug = 1;
            set_servo_target_step(&head.servos[SERVO_NECK], dir * RUB_NECK_STEP1, RUB_NECK_FAST_STEP);
            servo_target_debug = 0;
            if (abs(head.servos[SERVO_NECK].current - dir * RUB_NECK_STEP1) < RUB_NECK_FAST_STEP) {
                head.state.rub_state = RUB_MOVE2;
            }
            DBG2_PRINT("judge >> ",abs(head.servos[SERVO_NECK].current - dir * RUB_NECK_STEP1),RUB_NECK_FAST_STEP);
            DBG2_PRINT("move1 rub_neck = ",head.servos[SERVO_NECK].current,head.servos[SERVO_NECK].target);
            break;

        case RUB_MOVE2: // 第二步移动
            set_servo_target_step(&head.servos[SERVO_NECK], dir * RUB_NECK_FULL, RUB_NECK_FAST_STEP);

            if (abs(head.servos[SERVO_NECK].current - dir * RUB_NECK_FULL) < RUB_NECK_FAST_STEP) {
                head.state.rub_state = RUB_MOVE3;
            }
            break;

        case RUB_MOVE3: // 第三步移动(回退)
            set_servo_target_step(&head.servos[SERVO_NECK], dir * RUB_NECK_STEP1, RUB_NECK_SLOW_STEP);

            if (abs(head.servos[SERVO_NECK].current - dir * RUB_NECK_STEP1) < RUB_NECK_SLOW_STEP) {
                if (head.state.rub_count > 1) {
                    head.state.rub_count--;
                    head.state.rub_timer = RUB_HOLD_FRAMES;
                    head.state.rub_state = RUB_HOLD;
                } else {
                    // 准备恢复
                    head.state.rub_state = RUB_RECOVERY;
                }
            }
            break;

        case RUB_HOLD: // 多次蹭之间的停留
            if (--head.state.rub_timer >= 0) {
                head.state.rub_state = RUB_MOVE2; // 继续下一次蹭
            }
            break;

        case RUB_RECOVERY:
            set_servo_target_step(&head.servos[SERVO_NECK], 0, RUB_NECK_SLOW_STEP);
            if (abs(head.servos[SERVO_NECK].current) < RUB_NECK_SLOW_STEP) {
                head.state.rub_state = RUB_NONE;
            }
    }
    //DBG2_PRINT("end rub_neck = ",head.servos[SERVO_NECK].current,head.servos[SERVO_NECK].target);

    // 微笑表情
    //if (head.state.rub_smile && head.state.smile_type == SMILE_NONE) {
    //    head_smile(SMILE_SMALL);
    //}
}

// 主处理函数(每25ms调用一次)
void head_handler(void) {

    head.state.frame_count++;
    //DBG_PRINTF("count %d",head.state.frame_count);
    if(head.msg.flag)
        DBG2_PRINT("msg_flag = ",head.msg.flag,head.msg.type);
    // 0. 处理外部消息
    handle_msg();

    // 1. 处理高优先级行为
    handle_rub();
    handle_lick();
    handle_speak();

    // 2. 处理中优先级行为
    if(head.state.lick_state == LICK_NONE)
    {
        handle_smile();
    }
    else
    {
        DBG2_PRINT("lick ban smile lick = ",head.state.lick_state,LICK_NONE);
        head.state.smile_type = SMILE_NONE;
    }
    handle_blink();

    // 3. 处理低优先级行为(待机)
    if (head.state.rub_state == RUB_NONE &&
        head.state.lick_state == LICK_NONE &&
        head.state.smile_type == SMILE_NONE &&
        head.state.blink_type == BLINK_NONE) {
        handle_idle();
    }
    //else
    //{
    //    DBG_PRINTF("breath ban.");
    //}

    // 4. 更新所有舵机位置
    for (int i = 0; i < NUM_SERVOS; i++) {
        servo_update(&head.servos[i]);
    }

    if(head.state.speak_state)
    {
        DBG_PRINTF("speak_status = %d",get_speak_status());
        servo_show(SERVO_MOUTH);
    }

}

// ================= 外部调用接口 =================
// 眨眼控制
// type: BLINK_LEFT, BLINK_RIGHT, BLINK_BOTH
// count: 眨眼次数(1-255)
void head_blink(BlinkType type, uint8_t count) {
    DBG2_PRINT("blink >> ",type,count);
    head.state.blink_type = type;
    head.state.blink_count = count;
    head.state.blink_state = 0; // 开始眨眼

    // 重置眨眼计时器
    head.state.last_blink_time = head.state.frame_count;
    head.state.next_blink_interval = rand_range(BLINK_MIN_INTERVAL, BLINK_MAX_INTERVAL);
}

// 微笑控制
// type: SMILE_SMALL, SMILE_BIG
void head_smile(SmileType type) {
    DBG_PRINT("head_smile >> ",type);
    head.state.smile_type = type;
    head.state.smile_state = 0; // 开始微笑

    // 重置微笑计时器
    head.state.last_smile_time = 0;
}

// 讲话控制
// state: SPEAK_STOP, SPEAK_OPEN, SPEAK_CLOSE
// speed: 每帧最大移动量
// amplitude: 目标幅度
void head_speak(SpeakState state, uint16_t speed, int16_t amplitude) {
    DBG_PRINT("head_speak >> ",state);
    DBG2_PRINT("speed & amplitude = ",speed,amplitude);
    head.state.speak_state = state;
    head.state.speak_speed = speed;
    head.state.speak_amplitude = amplitude;
}

// 吐舌头控制
// neck_move: 是否晃动脖子(0/1)
void head_lick(uint8_t lick_mode,uint8_t neck_move) {
    DBG2_PRINT("head_lick >> ",lick_mode,neck_move);
    head.state.lick_state = LICK_OUT;
    head.state.lick_mode = lick_mode;
    head.state.neck_move = neck_move;
}

// 蹭控制
// dir: 方向(-1:左, 1:右)
// count: 蹭的次数(0表示持续)
// smile: 是否伴随微笑(0/1)
void head_rub(int8_t dir, uint8_t count, uint8_t smile) {
    head.state.rub_dir = (dir > 0) ? 1 : -1;
    head.state.rub_count = (count == 0) ? 255 : count; // 0表示无限
    head.state.rub_smile = smile;
    head.state.rub_state = RUB_MOVE1;
    DBG2_PRINT("head_rub >> ",head.state.rub_dir,head.state.rub_count);
}

// 停止蹭
void head_stop_rub(void) {
    head.state.rub_count = 1; // 设置为1次以便完成当前动作后停止
}

//
void handle_neck_start()
{
    head.state.neck_state = NECK_IDLE;
}

void handle_neck_stop()
{
    head.state.neck_state = NECK_NONE;
}


void head_flag_set(int idx,int flag)
{
    switch(idx)
    {
        case FLAG_BLINK:head.state.flag_auto_blink = flag;DBG_PRINTF("AUTO_WINK = %d",flag);break;
        case FLAG_SMILE:head.state.flag_auto_smile = flag;DBG_PRINTF("AUTO_SMILE = %d",flag);break;
    }
}

void head_msg_send(uint16_t type,uint16_t sz,int16_t data[])
{
    int i;
    if(head.msg.flag)
        return ;
    head.msg.flag = 1;
    head.msg.type = type;
    head.msg.sz = sz;
    for(i=0;i<sz;i++)
        head.msg.data[i] = data[i];
    head.msg.flag = 2;
    DBG2_PRINT("head_msg_send >>",type,sz);
}

void head_servo_send()
{
    int i;
    static count = 0;
    //DBG_PRINTF("servo send %d",count);
    ts15ctl_clear();
    for(i=0;i<NUM_SERVOS;i++)
    {
        ts15ctl_add(i+1,head.servos[i].current);
    }
    ts15ctl_make();
    //DBG_PRINTF("servo send end");
}
//=======================================================



int HeadFlag_Use = 0;
int HeadFlag_End = 0;

OS_SEM HeadSem;

void HeadMotion_SEMCreat()
{
    os_sem_create(&HeadSem, 0);
}

void HeadMotion_SEMSend()
{
    os_sem_set(&HeadSem, 0);
    os_sem_post(&HeadSem);
}

void HeadMotion_SEMPend()
{
    os_sem_pend(&HeadSem, 0);
}

static void HeadMotion_MainLoop(void *priv)
{
    head_init();
    while(HeadFlag_End==0)
    {
        HeadMotion_SEMPend();
        //to do啦。
        head_handler();
        head_servo_send();
    }
    HeadFlag_End = 2;
}

static void HeadMotion_OnTimer(void *priv)
{
    if(HeadFlag_Use)
        HeadMotion_SEMSend();
}

void robot_head_recover()
{
    robot_control_action_open_make();
    head_init();
}

void robot_head_timeset()
{
    ts15ctl_settimes(SERVO_UPDATE_MS*SERVO_AMPLIFICATION_FACTOR_TIME);//频率的SERVO_AMPLIFICATION_FACTOR倍
    ts15ctl_settime(SERVO_MOUTH,SERVO_UPDATE_MS*MOUTH_AMPLIFICATION_FACTOR_TIME);
    ts15ctl_settime(SERVO_L_EYELID,SERVO_UPDATE_MS*LID_AMPLIFICATION_FACTOR_TIME);
    ts15ctl_settime(SERVO_R_EYELID,SERVO_UPDATE_MS*LID_AMPLIFICATION_FACTOR_TIME);
}

void robot_head_read()
{
    //to do
    //准备做一个读取所有舵机数据，包括当前位置，目标位置，状态的所有数据。
    servo_read_all();
}
void HeadMotion_Setup()
{
    DBG_PRINTF("HeadMotion_Setup init = %d",head_init_flag);
    if(head_init_flag == 0)
    {
        HeadMotion_SEMCreat();
        sys_timer_add(NULL, HeadMotion_OnTimer, 25);//25毫秒发送一次舵机数据，每秒40帧
        thread_fork("HeadMotion_MainLoop", 28, 2048, 0, 0, HeadMotion_MainLoop, NULL);
        head_init_flag = 1;
    }
    else
    {
        head_init();
    }

}

void HeadMotion_Stop()
{
    if(HeadFlag_End==0)
    {
        HeadFlag_End = 1;
        while(HeadFlag_End!=2)
            ROBOT_DELAY_MS(5);
    }
    // to do
}

void robot_head_use_set(int use)
{
    HeadFlag_Use = use;
    //ts15ctl_settime(25*SERVO_AMPLIFICATION_FACTOR_TIME);//频率的SERVO_AMPLIFICATION_FACTOR倍
    DBG_PRINT("robot_head_use_set >>",use);
}

void robot_head_init(/*int use*/)
{
    HeadFlag_Use = 0;
    HeadMotion_Setup();
}

int get_head_status()
{
    if(head_init_flag!=1)
        return 0;
    return HeadFlag_Use;
}



