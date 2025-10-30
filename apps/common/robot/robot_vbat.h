#ifndef __ROBOT_VBAT_H__
#define __ROBOT_VBAT_H__


int robot_vbat_raw_data_get();
int robot_vbat_app_level_get();
int Robot_vBatLow_Flag_Get();
void robot_vbat_once();
void robot_vbat_init();


#endif // __ROBOT_VBAT_H__
