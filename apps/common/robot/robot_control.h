#ifndef __ROBOT_CONTROL_H__
#define __ROBOT_CONTROL_H__


int robot_control_once();
void robot_control_data_enqueue(uint8_t data[],int sz);
void robot_control_clean_enqueue();
int robot_control_communication_checksum(uint8_t *data,int sz);
void robot_control_command_send(uint8_t tag,uint8_t *data,int sz);
extern char deviceid[64];

#define CTRL_SEND robot_control_data_enqueue
#define CTRL_CLEAN_SEND robot_control_clean_enqueue

#define CTRL_CHECKSUM robot_control_communication_checksum
#define ACTION_DATA_SZ (20)
#define COMMAND_DATA_SZ (11)

enum
{
    PRESET_OPEN = 1,
    PRESET_WIFI_OK = 2,
    PRESET_WIFI_DISOK = 3,
    PRESET_BT_OK = 4,
    PRESET_BT_DISOK = 5,
    PRESET_APP_OK = 6,
    PRESET_APP_DISOK = 7,
};

#endif // __ROBOT_CONTROL_H__

