#ifndef __ROBOT_POSI_H__
#define __ROBOT_POSI_H__


extern uint16_t abote_distance;
extern uint16_t abote_irs;

uint16_t robot_posi_data_once(uint16_t debug,uint16_t mode);
uint16_t robot_program_get_robot_irdata(uint8_t*data1,uint8_t*data);
uint16_t robot_program_get_distance(uint8_t* data);
#endif // __ROBOT_POSI_H__
