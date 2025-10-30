#ifndef __ROBOT_ACTION_H__
#define __ROBOT_ACTION_H__

#define ROBOT_ACTION_SHOW_AUDIO_STOP (10)
#define ROBOT_ACTION_SHOW_AUDIO_ST (11)
#define ROBOT_ACTION_SHOW_AUDIO_SZ (29)

#define ROBOT_ACTION_SHOW_MUSIC_ST (40)
#define ROBOT_ACTION_SHOW_MUSIC_SZ (20)

#define ROBOT_ACTION_SHOW_EMOJI_ST (60)
#define ROBOT_ACTION_SHOW_EMOJI_SZ (100)

//#define ROBOT_ACTION_POSI_NULL  (0)
#define ROBOT_ACTION_POSI_STAND (0)
#define ROBOT_ACTION_POSI_SIT   (1)
#define ROBOT_ACTION_POSI_LIE   (2)
#define ROBOT_ACTION_POSI_SZ    (3)

#define ROBOT_ACTION_SYSTEM_IDX         (0)
#define ROBOT_ACTION_SYSTEM_OPEN        (1)
#define ROBOT_ACTION_SYSTEM_TRANS_ST    (1)

#define ROBOT_ACTION_MOVE_ST    (10)


#define ROBOT_ACTION_GROUP_ST   (20)
#define ROBOT_ACTION_GROUP_SZ   (30)

#define ROBOT_ACTION_STAND_ST   (ROBOT_ACTION_GROUP_ST)
#define ROBOT_ACTION_STAND_SZ   (ROBOT_ACTION_GROUP_SZ)
#define ROBOT_ACTION_SIT_ST     (ROBOT_ACTION_GROUP_ST+ROBOT_ACTION_GROUP_SZ)
#define ROBOT_ACTION_SIT_SZ     (ROBOT_ACTION_GROUP_SZ)
#define ROBOT_ACTION_LIE_ST     (ROBOT_ACTION_GROUP_ST+ROBOT_ACTION_GROUP_SZ*2)
#define ROBOT_ACTION_LIE_SZ     (ROBOT_ACTION_GROUP_SZ)

#define ROBOT_EMOJI_DOG_ST      (0)
#define ROBOT_EMOJI_SPE_ST      (40)

#define ROBOT_EMOJI_SPEED_LOW (1)
#define ROBOT_EMOJI_SPEED_DEFAULT (2)
#define ROBOT_EMOJI_SPEED_FAST (3)

int robot_action_data_make();
int robot_bus_ts15_idle();

void robot_action_data_saveplay(uint16_t type,uint16_t offset);
void robot_action_group_play(uint16_t idx);
void robot_action_system_group_play(uint16_t idx);

void robot_audio_play_break();
void robot_audio_voice_play(uint16_t audio_id);
void robot_audio_music_play(uint16_t music_id);
void robot_audio_tone_play(uint16_t type,uint16_t tone);

void robot_lcd_bus_data_send(uint16_t cmd,uint16_t addr,uint8_t *para,uint16_t psz);

void robot_lcd_mode_set(uint16_t mode);

void robot_lcd_emoji_set(uint16_t emoji_id);
void robot_lcd_emoji_play_playpause(uint16_t playplause);
void robot_lcd_emoji_set_speed(uint16_t speed);

void robot_lcd_char_set_ascii(int8_t ascii,uint8_t eyeid,uint8_t x,uint8_t y);
void robot_lcd_char_set_asciis(int8_t asciis[],int16_t sz,uint8_t eyeid,uint8_t x,uint8_t y);
uint16_t robot_lcd_char_set_num_int(int16_t num_i,uint8_t eyeid,uint16_t x,uint16_t y,uint16_t length,int mode);
uint16_t robot_lcd_char_set_num_double(double num_f,uint8_t eyeid,uint16_t x,uint16_t y,uint16_t length);
void robot_lcd_char_set_bcolour(uint8_t eyeid,uint8_t colour);
void robot_lcd_char_set_ccolour(uint8_t eyeid,uint8_t colour);
void robot_lcd_char_set_clr(uint8_t eyeid,uint16_t st,uint16_t ed);

void robot_action_program_play(uint16_t group_idx,uint16_t action_idx,uint16_t times);
void robot_action_cmd_play(uint16_t cmd,uint16_t times);
void robot_audio_cmd_play(int16_t audio,int16_t wait);
void robot_emoji_cmd_play(int16_t emoji);
void robot_action_stunt_random_play();
uint16_t robot_action_play_idx_get();
uint16_t robot_action_play_idx_get2();
void robot_remote_keep_once();
void robot_remote_keep_set(uint16_t keep);
void robot_remote_dir_set(uint16_t dir);

void robot_action_command_advance_play();
void advance_flag_check(void *priv);
void check_and_retract_if_needed(void);

extern int ts15_servo_start[];
extern int ts15_servo_dir[];
extern int ts15_servo_idx[];

#endif
