#include "robot_main.h"
#include "robot_debug.h"
#include "robot_sound.h"
#include "robot_adapter.h"

extern void Tbz_music_start(int tag);

int robot_musicdance_switch = 0;
int robot_musicdance_once_flag = 0;
int tbz_music_flag = 0;
int show_mdflag = 0;
int media_music_play_flag = 0;
extern int ai_music_flag;

void Tbz_musicdance_switch_set(int set,int save,int play)
{
    robot_musicdance_switch = set;
    void *func = NULL;
    if(save)
        Tbz_Config_Save(9,robot_musicdance_switch);
    if(play)
    {
        //if(ai_music_flag)
        //    func = __this->dec_ops->dec_breakpoint;
        if(robot_musicdance_switch)
        {
            //os_set_play_audio_nobreak("md_on.mp3", NULL);
            os_set_play_audio_resume("md_on.mp3",1);
            //if(ai_music_flag)
            //{
            //    Tbz_music_start(turning_music_flag);
            //}
            if(os_get_bt_ops_status())
            {
                Tbz_music_start(1);
            }
        }
        else
        {
            os_set_play_audio_resume("md_off.mp3",1);
            //if(ai_music_flag)
                //Action_set(Sit_down2);
        }
    }
    robot_main_debug_output("robot_musicdance_switch = ",robot_musicdance_switch);
}


void Tbz_music_start(int tag)
{
    //Action_set2(Sit_down2);
    robot_main_debug_output("robot_musicdance_switch = ",robot_musicdance_switch);
    if(robot_musicdance_switch)
    {
        if((tag==1)||(tag==2))//儿歌故事都要启动随音起舞
        {
            robot_musicdance_once_flag = 1;
            media_music_play_flag = 1;
        }
        else
        {
            robot_musicdance_once_flag = 0;
            media_music_play_flag = 0;
        }
        robot_main_debug_output("robot_musicdance_start = ",robot_musicdance_once_flag);
    }
    tbz_music_flag = 1;
    //robot_emoji_emoji_set(EMOJI_MUSIC,0);
}
