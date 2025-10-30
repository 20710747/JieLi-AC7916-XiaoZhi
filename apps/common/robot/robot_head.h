#ifndef __ROBOT_HEAD_H__
#define __ROBOT_HEAD_H__


#define NUM_SERVOS          9   // �������

#define SERVO_UPDATE_MS     25  // �����������(ms)
#define PI                  3.14159265358979323846

#if 0
#define SERVO_AMPLIFICATION_FACTOR_MOVE 4  //����Ŵ�����
#define SERVO_AMPLIFICATION_FACTOR_TIME 2
#else
#define SERVO_AMPLIFICATION_FACTOR_MOVE 16  //����Ŵ�����
#define SERVO_AMPLIFICATION_FACTOR_TIME 6
#define MOUTH_AMPLIFICATION_FACTOR_TIME 4
#define LID_AMPLIFICATION_FACTOR_TIME 8
#endif
// �����������
typedef enum {
    SERVO_NECK,         // ����
    SERVO_MOUTH,        // ���
    SERVO_EYES,         // �۾�
    SERVO_L_EYELID,     // ����Ƥ
    SERVO_R_EYELID,     // ����Ƥ
    SERVO_L_CHEEK,      // ����
    SERVO_R_CHEEK,      // ����
    SERVO_TONGUE,       // ��ͷ
    SERVO_SPARE         // ����
} ServoIndex;

// ================= ��Ϊ�����궨�� =================
// ����˶�����
#define SERVO_MIN_POS       -2000
#define SERVO_MAX_POS       2000
#define DEFAULT_STEP        10

// �����˶�����
#define EYE_RANGE      65     // ��63 (ԭ50)

#define EYE_STEP_MAX       3      // ԭ2
#define EYE_MOVE_PROB      20     // ÿ֡�ƶ�����(��)
#define EYE_MOVE_PROB2     60     // ��̬ʱÿ֡�ƶ�����

#define EYELID_MOVE_PROB   40     // ԭ40

#define BREATH_CHEEK_STEP_MAX 6
// �����˶�����
#define NECK_RANGE_HALF      100    // ��100 (ԭ80)
#define NECK_RANGE_MIN       50     // ԭ40
#define NECK_RANGE_MAX       125    // ԭ100

#define NECK_STEP_MIN       2      // ԭ1 (���ֲ���)
#define NECK_STEP_MAX       3      // ԭ2
#define NECK_MIN_MOVE_FRAMES 25    // ��С�ƶ���� (ԭ20)

// ��Ƥ�˶�����
//#define EYELID_RANGE        70     // ��75 (ԭ60)
#define EYELID_STEP_MAX     6      // ԭ5
#define EYELID_DIFF_MAX     3     // ���۲������ֵ (ԭ10)

// ��������
#define BREATH_MOUTH_RANGE  25     // ԭ20
#define BREATH_CHEEK_RANGE  120    // ԭ120
#define BREATH_LID_RANGE    40
#define BREATH_LID_BASE     -25
#define BREATH_CYCLE_FRAMES 250    // ��������֡�� (ԭ200)

// գ�۲���
#define BLINK_MIN_INTERVAL          200    // (ԭ160)
#define BLINK_MAX_INTERVAL          250    // (ԭ200)

#define BLINK_CLOSE_POS             160     // (ԭ80)
#define BLINK_CLOSE_STEP_BOTH       60     // (ԭ40)
#define BLINK_CLOSE_STEP_SINGLE     60     // (ԭ15)
#define BLINK_CLOSE_STEP_SLOW       40     // (ԭ15)


#define BLINK_OPEN_POS              -20     // (ԭ80)
#define BLINK_OPEN_STEP_BOTH        80     // (ԭ30)
#define BLINK_OPEN_STEP_SINGLE      80     // (ԭ20)
#define BLINK_OPEN_STEP_SLOW        60     // (ԭ20)


#define BLINK_ANTHOER_POS           10
#define BLINK_ANTOHER_STEP          10

#define BLINK_CHEEK_PULL_POS        300    // (ԭ450)
#define BLINK_CHEEK_PULL_SLOW_POS   450    // (ԭ450)
#define BLINK_CHEEK_STEP            10     // (ԭ20)

#define BLINK_CHEEK_RECOVER_POS     100
#define BLINK_CHEEK_RECOVERY_STEP   40     // (ԭ40)

#define SINGLE_BLINK_PROB           750
#define DOUBLE_BLINK_PROB           250
#define BLINK_HOLD_FRAMES_BOTH      2
#define BLINK_HOLD_FRAMES_SINGLE    2
#define BLINK_HOLD_FRAMES_SLOW      20
#define BLINK_INTERVAL_FRAMES       4
#define BLINK_STOP_FRAMES           2








// ΢Ц����
#if 0
#define SMILE_MIN_INTERVAL  2000    // (ԭ1600)
#define SMILE_MAX_INTERVAL  3500    // (ԭ2800)
#else
#define SMILE_MIN_INTERVAL  480     // (ԭ480)
#define SMILE_MAX_INTERVAL  600     // (ԭ600)
#endif

#define SMILE_MOUTH_OPEN    38      // (ԭ30)
#define SMILE_MOUTH_STEP    3       // (ԭ2)
#define SMILE_MOUTH_CLOSE   0       // (ԭ0)
#define SMILE_MOUTH_CLOSE_STEP 5    // (ԭ4)
#define SMILE_BIG_CHEEK     563     // (ԭ450)
#define SMILE_SMALL_CHEEK   438     // (ԭ350)
#define SMILE_CHEEK_STEP    8       // (ԭ6)
#define SMILE_EYE_CLOSE     100     // (ԭ80)
#define SMILE_EYE_STEP      3       // (ԭ2)
#define SMILE_RECOVERY_STEP 5       // (ԭ4)
#define SMILE_BIG_HOLD_FRAMES   13  // (ԭ10)
#define SMILE_SMALL_HOLD_FRAMES   6 // (ԭ5)
#define BIG_SMILE_PROB      938     // (ԭ750)

// ��������
#define SPEAK_MOUTH_MIN     75      // (ԭ60)
#define SPEAK_MOUTH_MAX     113     // (ԭ90)
#define SPEAK_MOUTH_STEP    13      // (ԭ10)
#define SPEAK_MOUTH_FULL    50      // (ԭ40)
#define SPEAK_TONGUE_MIN    100     // (ԭ80)
#define SPEAK_TONGUE_MAX    125     // (ԭ100)
#define SPEAK_TONGUE_STEP   5       // (ԭ4)
#define SPEAK_CHEEK_OPEN    160     // (ԭ180)
#define SPEAK_CHEEK_CLOSE   80      // (ԭ120)
#define SPEAK_CHEEK_STEP    8       // (ԭ8)
#define SPEAK_NECK_RANGE    250     // ��250 (ԭ200)
#define SPEAK_NECK_STEP     5       // (ԭ4)
#define SPEAK_MOUTH_CLOSE   -25     // (ԭ-20)

#define SPEAK_EYELID_OPEN_MIN -50   // (ԭ-40)
#define SPEAK_EYELID_OPEN_MAX 38    // (ԭ30)

#define SPEAK_EYELID_CLOSE_MIN 50   // (ԭ40)
#define SPEAK_EYELID_CLOSE_MAX 75   // (ԭ60)

#define SPEAK_EYELID_STEP   10      // (ԭ8)

// ����ͷ����
#define LICK_MOUTH_OPEN_MODE1   100  // (ԭ80)
#define LICK_MOUTH_OPEN_MODE2   125  // (ԭ100)
#define LICK_MOUTH_OPEN_MODE3   150  // (ԭ120)
#define LICK_MOUTH_OPEN_MODE4   163  // (ԭ130)
#define LICK_MOUTH_FULL         175  // (ԭ140)
#define LICK_MOUTH_STEP         13   // (ԭ10)
#define LICK_TONGUE_OUT         313  // (ԭ250)
#define LICK_TONGUE_STEP        25   // (ԭ20)
#define LICK_TONGUE_RETRACT_STEP 50  // (ԭ40)
#define LICK_NECK_RANGE         375  // ��375 (ԭ300)
#define LICK_NECK_STEP          12   // (ԭ10)
#define LICK_HOLD_FRAMES        40   // (ԭ40)

// �����
#define RUB_NECK_STEP1      225     // (ԭ180)
//#define RUB_NECK_STEP2      175     // (ԭ140)
#define RUB_NECK_FULL       400     // (ԭ320)
#define RUB_NECK_FAST_STEP  5       // (ԭ4)
#define RUB_NECK_SLOW_STEP  4       // (ԭ3)
#define RUB_HOLD_FRAMES     50      // (ԭ40)

#define RANGE_MAX           6

// ================= ���ݽṹ���� =================
typedef struct {
    int16_t current;    // ��ǰֵ
    int16_t target;     // Ŀ��ֵ
    int16_t step;      // ÿ֡���仯��
    int16_t send;       // ʵ�ʸ�TS15���͵�ֵ
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
    // �������
    Servo servos[NUM_SERVOS];

    // ��Ϊ״̬
    struct {
        uint32_t frame_count;
        // �������
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

        // գ�����
        BlinkType blink_type;
        int8_t blink_count;
        uint8_t blink_state;
        int8_t blink_timer;

        // ΢Ц���
        SmileType smile_type;
        uint8_t smile_state;
        int8_t smile_timer;

        // �������
        SpeakState speak_state;
        uint16_t speak_speed;
        int16_t speak_amplitude;

        // ����ͷ���
        LickState lick_state;
        uint8_t lick_mode;
        uint8_t lick_timer;
        uint8_t neck_move;

        // �����
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
