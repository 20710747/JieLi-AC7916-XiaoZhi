#ifndef __ROBOT_ADAPTER_H__
#define __ROBOT_ADAPTER_H__

#define RANDOM(M) robot_random(M)


#define ROBOT_DELAY_US(x) os_set_delay_us((x))
#define ROBOT_DELAY_MS(x) os_set_delay_ms((x))

#define ROBOT_GETTIME_MS() os_get_time_ms()


void os_set_play_audio(const char *name,void *play_end);
void os_set_play_audio_resume(const char *name,int resume);
int os_get_play_status(void);
int os_set_play_break(void);
int os_get_tone_status();
void os_set_tone_status(int tone);
void os_set_tone_ban(int ban);
void os_set_volume(int volume,int play,int save);
void os_set_volume_resume();
int os_get_volume();

//配置网络
/*
-1:改变当前状态
0：退出连接状态
1: 进入连接状态
*/

void os_set_net_config_mode(int mode);
int os_get_net_config_mode();
int os_set_net_config_info(char *ssid,char *pwd);
int os_get_net_connect();
int os_get_bt_ops_status();
int os_set_delay_ms(int ms);

int os_set_delay_us(int ms);
uint32_t os_get_time_ms();

void llm_set_interactive_mode(int mode);
int llm_get_interactive_mode();

#endif // __ROBOT_ADAPTER_H__
