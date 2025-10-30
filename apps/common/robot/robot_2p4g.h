#ifndef __ROBOT_2P4G_H__
#define __ROBOT_2P4G_H__
void robot_2p4g_once();
//ÓÃÓÚ±à³ÌÄ£¿é
void robot_2p4g_program_data_send(uint8_t idx,uint8_t data);
void robot_2p4g_program_data_set(uint8_t idx,uint8_t data,uint8_t program_data[]);

void robot_2p4g_TestData_Send(uint16_t type);
void robot_2p4g_switch_set(uint16_t sw);

void robot_2p4g_data_make(uint8_t *data,int sz);
int robot_2p4g_remote_make(uint8_t cmd);
void robot_2p4g_system_make(uint8_t cmd);
void robot_2p4g_remote_stop();
#endif // __ROBOT_2P4G_H__
