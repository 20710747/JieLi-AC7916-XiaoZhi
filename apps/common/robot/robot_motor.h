#ifndef __ROBOT_MOTOR_H__
#define __ROBOT_MOTOR_H__

#define ROBOT_MOTOR_CMD_SZ 6

void robot_motor_init();
void robot_motor_stop();
uint16_t robot_motor_cmdGo(uint8_t *cmd);
void robot_motor_exit();


typedef struct motor_ctrl_type{
    int id;
    int gpio;
	int type;
	int target_speed;
	int current_speed;
	int speed_level;
	int speed_dir;

}MOTOR_CTRL;

#endif
