#ifndef __ROBOT_IR_FOLLOW_H__
#define __ROBOT_IR_FOLLOW_H__

#define IR_CAP_DEBUG 1
#define ROBOT_IR_TYPE_FOLLOW 0
#define ROBOT_IR_TYPE_TOUCH  4
uint16_t robot_irfollow_switch_get();
void robot_irfollow_switch_set(uint16_t set);
void robot_ir_timer_once();//20ms

void robot_ir_state_add(uint16_t set);
void robot_irfollow_init();

void robot_irobs_switch_set(uint16_t set);
void robot_irobs_timer_once();//20ms
void robot_irobs_init();
void robot_irobs_data_show();
void robot_irobs_send_init();
#if IR_CAP_DEBUG
void robot_irdebug_cap_init();
void robot_irdebug_cap_add(uint16_t cap);
void robot_irdebug_cap_show();
#endif
void robot_irfollow_adjust_ok();
uint16_t robot_irobs_test_once();
uint16_t robot_irobs_test();
#endif // __ROBOT_IR_FOLLOW_H__
