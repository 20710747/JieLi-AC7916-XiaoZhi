#ifndef __ROBOT_CONFIG_H__
#define __ROBOT_CONFIG_H__

#define TBZ_ABOTE_VERSION_TAG "TBZ_ABOTE_BRAIN"

#define TBZ_TURNING_VERSION "0.1.0"
#define TBZ_ABOTE_DATE "2024-04-10_1"

#define TBZ_ABOTE_VERSION_A (1)
#define TBZ_ABOTE_VERSION_B (4)
#define TBZ_ABOTE_VERSION_C (8)

extern int action_flag;
extern volatile int action_code;
extern volatile int next_action_code;

extern void Tbz_Config_Save(int cfg, int data);

extern int ROBOT_SWITCH_ASRWAKEUP;
extern int ROBOT_DATA_LANG;
extern int ROBOT_DATA_VCN;
extern int ROBOT_DATA_PERSONA;
extern int ROBOT_DATA_VOLUME;

#endif // __ROBOT_CONFIG_H__


