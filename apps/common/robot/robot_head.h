#ifndef __ROBOT_HEAD_H__
#define __ROBOT_HEAD_H__


#define NUM_SERVOS          9   // 舵机总数

#define SERVO_UPDATE_MS     25  // 舵机更新周期(ms)
#define PI                  3.14159265358979323846

#if 0
#define SERVO_AMPLIFICATION_FACTOR_MOVE 4  //舵机放大因子
#define SERVO_AMPLIFICATION_FACTOR_TIME 2
#else
#define SERVO_AMPLIFICATION_FACTOR_MOVE 16  //舵机放大因子
#define SERVO_AMPLIFICATION_FACTOR_TIME 6
#define MOUTH_AMPLIFICATION_FACTOR_TIME 4
#define LID_AMPLIFICATION_FACTOR_TIME 8
#endif
// 舵机索引定义
typedef enum {
    SERVO_NECK,         // 脖子
    SERVO_MOUTH,        // 嘴巴
    SERVO_EYES,         // 眼睛
    SERVO_L_EYELID,     // 左眼皮
    SERVO_R_EYELID,     // 右眼皮
    SERVO_L_CHEEK,      // 左脸
    SERVO_R_CHEEK,      // 右脸
    SERVO_TONGUE,       // 舌头
    SERVO_SPARE         // 备用
} ServoIndex;

// ================= 行为参数宏定义 =================
// 舵机运动参数
#define SERVO_MIN_POS       -2000
#define SERVO_MAX_POS       2000
#define DEFAULT_STEP        10

// 眼球运动参数
#define EYE_RANGE      65     // ±63 (原50)

#define EYE_STEP_MAX       3      // 原2
#define EYE_MOVE_PROB      20     // 每帧移动概率(‰)
#define EYE_MOVE_PROB2     60     // 静态时每帧移动概率

#define EYELID_MOVE_PROB   40     // 原40

#define BREATH_CHEEK_STEP_MAX 6
// 脖子运动参数
#define NECK_RANGE_HALF      100    // ±100 (原80)
#define NECK_RANGE_MIN       50     // 原40
#define NECK_RANGE_MAX       125    // 原100

#define NECK_STEP_MIN       2      // 原1 (保持不变)
#define NECK_STEP_MAX       3      // 原2
#define NECK_MIN_MOVE_FRAMES 25    // 最小移动间隔 (原20)

// 眼皮运动参数
//#define EYELID_RANGE        70     // ±75 (原60)
#define EYELID_STEP_MAX     6      // 原5
#define EYELID_DIFF_MAX     3     // 两眼差异最大值 (原10)

// 呼吸参数
#define BREATH_MOUTH_RANGE  25     // 原20
#define BREATH_CHEEK_RANGE  120    // 原120
#define BREATH_LID_RANGE    40
#define BREATH_LID_BASE     -25
#define BREATH_CYCLE_FRAMES 250    // 呼吸周期帧数 (原200)

// 眨眼参数
#define BLINK_MIN_INTERVAL          200    // (原160)
#define BLINK_MAX_INTERVAL          250    // (原200)

#define BLINK_CLOSE_POS             160     // (原80)
#define BLINK_CLOSE_STEP_BOTH       60     // (原40)
#define BLINK_CLOSE_STEP_SINGLE     60     // (原15)
#define BLINK_CLOSE_STEP_SLOW       40     // (原15)


#define BLINK_OPEN_POS              -20     // (原80)
#define BLINK_OPEN_STEP_BOTH        80     // (原30)
#define BLINK_OPEN_STEP_SINGLE      80     // (原20)
#define BLINK_OPEN_STEP_SLOW        60     // (原20)


#define BLINK_ANTHOER_POS           10
#define BLINK_ANTOHER_STEP          10

#define BLINK_CHEEK_PULL_POS        300    // (原450)
#define BLINK_CHEEK_PULL_SLOW_POS   450    // (原450)
#define BLINK_CHEEK_STEP            10     // (原20)

#define BLINK_CHEEK_RECOVER_POS     100
#define BLINK_CHEEK_RECOVERY_STEP   40     // (原40)

#define SINGLE_BLINK_PROB           750
#define DOUBLE_BLINK_PROB           250
#define BLINK_HOLD_FRAMES_BOTH      2
#define BLINK_HOLD_FRAMES_SINGLE    2
#define BLINK_HOLD_FRAMES_SLOW      20
#define BLINK_INTERVAL_FRAMES       4
#define BLINK_STOP_FRAMES           2








// 微笑参数
#if 0
#define SMILE_MIN_INTERVAL  2000    // (原1600)
#define SMILE_MAX_INTERVAL  3500    // (原2800)
#else
#define SMILE_MIN_INTERVAL  480     // (原480)
#define SMILE_MAX_INTERVAL  600     // (原600)
#endif

#define SMILE_MOUTH_OPEN    38      // (原30)
#define SMILE_MOUTH_STEP    3       // (原2)
#define SMILE_MOUTH_CLOSE   0       // (原0)
#define SMILE_MOUTH_CLOSE_STEP 5    // (原4)
#define SMILE_BIG_CHEEK     563     // (原450)
#define SMILE_SMALL_CHEEK   438     // (原350)
#define SMILE_CHEEK_STEP    8       // (原6)
#define SMILE_EYE_CLOSE     100     // (原80)
#define SMILE_EYE_STEP      3       // (原2)
#define SMILE_RECOVERY_STEP 5       // (原4)
#define SMILE_BIG_HOLD_FRAMES   13  // (原10)
#define SMILE_SMALL_HOLD_FRAMES   6 // (原5)
#define BIG_SMILE_PROB      938     // (原750)

// 讲话参数
#define SPEAK_MOUTH_MIN     75      // (原60)
#define SPEAK_MOUTH_MAX     113     // (原90)
#define SPEAK_MOUTH_STEP    13      // (原10)
#define SPEAK_MOUTH_FULL    50      // (原40)
#define SPEAK_TONGUE_MIN    100     // (原80)
#define SPEAK_TONGUE_MAX    125     // (原100)
#define SPEAK_TONGUE_STEP   5       // (原4)
#define SPEAK_CHEEK_OPEN    160     // (原180)
#define SPEAK_CHEEK_CLOSE   80      // (原120)
#define SPEAK_CHEEK_STEP    8       // (原8)
#define SPEAK_NECK_RANGE    250     // ±250 (原200)
#define SPEAK_NECK_STEP     5       // (原4)
#define SPEAK_MOUTH_CLOSE   -25     // (原-20)

#define SPEAK_EYELID_OPEN_MIN -50   // (原-40)
#define SPEAK_EYELID_OPEN_MAX 38    // (原30)

#define SPEAK_EYELID_CLOSE_MIN 50   // (原40)
#define SPEAK_EYELID_CLOSE_MAX 75   // (原60)

#define SPEAK_EYELID_STEP   10      // (原8)

// 吐舌头参数
#define LICK_MOUTH_OPEN_MODE1   100  // (原80)
#define LICK_MOUTH_OPEN_MODE2   125  // (原100)
#define LICK_MOUTH_OPEN_MODE3   150  // (原120)
#define LICK_MOUTH_OPEN_MODE4   163  // (原130)
#define LICK_MOUTH_FULL         175  // (原140)
#define LICK_MOUTH_STEP         13   // (原10)
#define LICK_TONGUE_OUT         313  // (原250)
#define LICK_TONGUE_STEP        25   // (原20)
#define LICK_TONGUE_RETRACT_STEP 50  // (原40)
#define LICK_NECK_RANGE         375  // ±375 (原300)
#define LICK_NECK_STEP          12   // (原10)
#define LICK_HOLD_FRAMES        40   // (原40)

// 蹭参数
#define RUB_NECK_STEP1      225     // (原180)
//#define RUB_NECK_STEP2      175     // (原140)
#define RUB_NECK_FULL       400     // (原320)
#define RUB_NECK_FAST_STEP  5       // (原4)
#define RUB_NECK_SLOW_STEP  4       // (原3)
#define RUB_HOLD_FRAMES     50      // (原40)

#define RANGE_MAX           6

// ================= 数据结构定义 =================
typedef struct {
    int16_t current;    // 当前值
    int16_t target;     // 目标值
    int16_t step;      // 每帧最大变化量
    int16_t send;       // 实际给TS15发送的值
} Servo;

typedef enum {
    MSG_NONE,
    MSG_BLINK,
    MSG_SMILE,
    MSG_SPEAK,
    MSG_LICK,
    MSG_RUB,
    MSG_SET,
    MSG_DEBUG,
} MsgType;

typedef enum {
    BLINK_NONE,
    BLINK_LEFT,
    BLINK_RIGHT,
    BLINK_BOTH,
    BLINK_LEFT_SLOW,
    BLINK_RIGHT_SLOW,
    BLINK_COUNT,
} BlinkType;

typedef enum {
    SMILE_NONE,
    SMILE_SMALL,
    SMILE_BIG,
    SMILE_AFTERSPEAK,
} SmileType;

typedef enum {
    SPEAK_NONE,
    SPEAK_OPEN,
    SPEAK_CLOSE,
    SPEAK_OPEN_RUN,
    SPEAK_CLOSE_RUN,
} SpeakState;

typedef enum {
    LICK_NONE,
    LICK_OUT,
    LICK_HOLD,
    LICK_IN
} LickState;

typedef enum {
    RUB_NONE,
    RUB_MOVE1,
    RUB_MOVE2,
    RUB_MOVE3,
    RUB_HOLD,
    RUB_RECOVERY
} RubState;

typedef enum {
    NECK_NONE,
    NECK_IDLE,
    NECK_SPEAK,
    NECK_LICK,
    NECK_RUN,
}NeckState;

typedef enum {
    FLAG_NONE = 0,
    FLAG_BLINK = 1,
    FLAG_SMILE = 2,
}AutoFlag;

enum
{
    MSG_DEBUG_NONE = 0,
    MSG_DEBUG_NECKTIME,
    MSG_DEBUG_NECTSET,
};
typedef struct {
    // 舵机数组
    Servo servos[NUM_SERVOS];

    // 行为状态
    struct {
        uint32_t frame_count;
        // 待机相关
        uint32_t last_blink_time;
        uint32_t next_blink_interval;
        uint32_t last_smile_time;
        uint32_t next_smile_interval;
        uint16_t breath_counter;

        int16_t flag_auto_blink;
        int16_t flag_auto_smile;

        NeckState neck_state;
        int16_t neck_timer;
        int16_t neck_target;
        int16_t neck_step;
        int16_t neck_dir;

        // 眨眼相关
        BlinkType blink_type;
        int8_t blink_count;
        uint8_t blink_state;
        int8_t blink_timer;

        // 微笑相关
        SmileType smile_type;
        uint8_t smile_state;
        int8_t smile_timer;

        // 讲话相关
        SpeakState speak_state;
        uint16_t speak_speed;
        int16_t speak_amplitude;

        // 吐舌头相关
        LickState lick_state;
        uint8_t lick_mode;
        uint8_t lick_timer;
        uint8_t neck_move;

        // 蹭相关
        RubState rub_state;
        int8_t rub_dir;
        uint8_t rub_count;
        int8_t rub_timer;
        uint8_t rub_smile;
    } state;
    struct
    {
        uint16_t flag;
        uint16_t sz;
        uint16_t type;
        int16_t data[15];
    }msg;
} HeadController;


typedef struct
{
    int16_t lid_check,lid_pos,lid_step;
    int16_t cheek_set,cheek_pos,cheek_step;
}BlinkPosData;

typedef struct
{
    int16_t type;
    BlinkPosData close_left,open_left;
    BlinkPosData close_right,open_right;
    int16_t hold_frame,interval_frame,stop_frame;
}BlinkData;


void head_blink(BlinkType type, uint8_t count);
void head_smile(SmileType type);
void head_rub(int8_t dir, uint8_t count, uint8_t smile);
void head_speak(SpeakState state, uint16_t speed, int16_t amplitude);
void head_lick(uint8_t lick_mode,uint8_t neck_move);
void head_flag_set(int idx,int flag);
void handle_neck_start();
void handle_neck_stop();
#endif // __ROBOT_HEAD_H__
