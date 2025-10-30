#ifndef __ROBOT_APP_H__
#define __ROBOT_APP_H__


#define APP_SEND robot_app_data_send


void robot_app_debug_mode_set(int set);
void robot_app_debug_mode_switch();
uint16_t robot_app_data_check(uint8_t *data,uint8_t len);
uint16_t robot_app_data_send(uint8_t  * data, uint16_t sz);

void tbz_app_command_send(uint8_t tag,uint8_t *data,int sz);
void tbz_app_action_send(uint8_t *data,int sz);
int robot_control_command_holdcmd(int flag, uint8_t *data);

#endif // __ROBOT_APP_H__

