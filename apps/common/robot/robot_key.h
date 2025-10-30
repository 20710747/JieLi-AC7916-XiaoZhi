#ifndef __ROBOT_KEY_H__
#define __ROBOT_KEY_H__

#define KEY_IDLE 0
#define KEY_HOLD 1
#define KEY_LONG 2
#define KEY_DOWN 3
#define KEY_UP 4

void robot_key_init();
void robot_key_switch_set(uint16_t set);
void robot_key_test_once();
void robot_key_timer_once();
uint16_t robot_key_get_status(uint16_t status[]);
uint16_t robot_key_get_keysize();
void robot_key_data_show();
#endif // __ROBOT_KEY_H__


