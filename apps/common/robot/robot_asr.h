#ifndef __TEBOZ_ASR_H__
#define __TEBOZ_ASR_H__

void robot_asr_switch_set(uint16_t sw);
void robot_asr_set_init();
uint16_t robot_asr_get_ready();
void robot_asr_set_recog_break();
void robot_asr_set_recog_once();
void robot_asr_word_set(uint16_t widx,char word[]);
void robot_asr_set_findnoise(uint16_t flag);
void robot_asr_word_make();
void robot_asr_statusflag_set(uint16_t flag);

#define ROBOT_ASR_FINDNOISE 3
#define ROBOT_ASR_ADJUSTNOISE 4
#define ROBOT_ASR_RECORD 8
#endif // __TEBOZ_ASR_H__
