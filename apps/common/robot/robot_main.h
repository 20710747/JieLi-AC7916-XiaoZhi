#ifndef __TBZ_ROBOT_H__
#define __TBZ_ROBOT_H__

#include "system/includes.h"
#include "syscfg/config_interface.h"
#include "os/os_api.h"
#include "asm/uart_dev.h"
#include "asm/crc16.h"
#include "device/device.h"
#include "device/uart.h"
#include "app_config.h"
#include "gpio.h"
#include "time.h"

#define MAININPUT_MODE_CMD 1
#define MAININPUT_MODE_DATA 2
#define MAININPUT_MODE_CONFIG 3
#define MAININPUT_MODE_APP 4
#define MAININPUT_MODE_RF2P4G 5
#define MAININPUT_MODE_UARTBUS 6
#define MAININPUT_MODE_MOTOR 7
#define MAININPUT_MODE_SSID_PWD 8
#define MAININPUT_MODE_SERVO 9
#define MAININPUT_MODE_HEAD 10


uint8_t robot_data_checksum_uint8t(uint8_t *data,uint16_t sz);
uint8_t robot_data_checksum_flash(uint8_t *data,uint16_t sz);
uint16_t robot_random(uint16_t m);
extern void os_set_play_audio(const char *fname, void *dec_end_handler);
void robot_main_breathe_set(uint16_t cs);
#define NUMBER_IN(A,B,C) ((A>=B)&&(A<=C))


extern int aiui_get_vcn(void);
extern void aiui_set_vcn(int idx);
extern int aiui_get_language(void);
extern void aiui_set_language(int idx);
extern int aiui_get_persona(void);
extern  void aiui_set_persona(int idx);

extern void aiui_set_devid(char *devid);
extern char *aiui_get_devid();

uint32_t robot_main_get_ms();
void robot_main_timer_init();
void robot_main_wait_set(int waitms);
void robot_main_wait_do();
#endif // __TBZ_ROBOT_H__
