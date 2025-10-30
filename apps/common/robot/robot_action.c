// robot action
#include "robot_main.h"
#include "robot_debug.h"
#include "robot_uart.h"
#include "robot_adapter.h"
#include "robot_bus.h"
#include "robot_control.h"
#include "robot_action.h"
#include "robot_storage.h"
#include "robot_posi.h"
#include <stdint.h>

#define NEW_PROTOTYPE 0
#define TS_RW_DEBUG 0

#define ACTION_ALL (1)
#define ACTION_HALF (2)
#define ACTION_NONE (3)

#include <time.h>

extern int speak_face_flag;
extern int speak_tone_flag;
extern int speak_eye_flag;

// 无
// 脖子 嘴巴 眼睛 左眼皮
// 右眼皮 左表情 右表情 舌头
// 夹吸（备用）
#if NEW_PROTOTYPE
int ts15_servo_idx[]=
{
    0,
    5,1,3,4,
    8,2,6,7,
    9,
};

#define SERVO_NECK_BASE         150 // 脖子
#define SERVO_MOUTH_BASE        154 // 嘴巴
#define SERVO_EYES_BASE         150 // 眼睛
#define SERVO_L_EYELID_BASE     165 // 左眼皮
#define SERVO_R_EYELID_BASE     135 // 右眼皮
#define SERVO_L_CHEEK_BASE      160 // 左脸
#define SERVO_R_CHEEK_BASE      140 // 右脸
#define SERVO_TONGUE_BASE       150 // 舌头
#define SERVO_SPARE_BASE        150 // 备用

#else
int ts15_servo_idx[]=
{
    0,
    5,1,3,4,
    7,2,6,8,
    9,
};

#define SERVO_NECK_BASE         150 // 脖子
#define SERVO_MOUTH_BASE        152 // 嘴巴
#define SERVO_EYES_BASE         150 // 眼睛
#define SERVO_L_EYELID_BASE     160 // 左眼皮
#define SERVO_R_EYELID_BASE     230 // 右眼皮
#define SERVO_L_CHEEK_BASE      140 // 左脸
#define SERVO_R_CHEEK_BASE      165 // 右脸
#define SERVO_TONGUE_BASE       100 // 舌头
#define SERVO_SPARE_BASE        150 // 备用
#endif
uint16_t ts15_data_init[]=
{
    0,
    SERVO_NECK_BASE, SERVO_MOUTH_BASE, SERVO_EYES_BASE,
    SERVO_L_EYELID_BASE, SERVO_R_EYELID_BASE,
    SERVO_L_CHEEK_BASE, SERVO_R_CHEEK_BASE,
    SERVO_TONGUE_BASE,SERVO_SPARE_BASE,
};

#define TS15_ANG2BASE_DATA(ang) (((ang+15)*4096)/330)
uint16_t ts15_servo_base[] = {
    0,      // 占位
    TS15_ANG2BASE_DATA(SERVO_NECK_BASE),
    TS15_ANG2BASE_DATA(SERVO_MOUTH_BASE),
    TS15_ANG2BASE_DATA(SERVO_EYES_BASE),
    TS15_ANG2BASE_DATA(SERVO_L_EYELID_BASE),
    TS15_ANG2BASE_DATA(SERVO_R_EYELID_BASE),
    TS15_ANG2BASE_DATA(SERVO_L_CHEEK_BASE),
    TS15_ANG2BASE_DATA(SERVO_R_CHEEK_BASE),
    TS15_ANG2BASE_DATA(SERVO_TONGUE_BASE),
    TS15_ANG2BASE_DATA(SERVO_SPARE_BASE),
};

int ts15_servo_dir[]=
{
	0,
	1,-1,1,-1,
	1,1,-1,-1,
	1,
};

// 获取数组元素数量
#define TS15_SERVO_COUNT (sizeof(ts15_servo_base) / sizeof(ts15_servo_base[0]))



int face_data_mid = 0;

const int TSMoveMode[]=
{
	1,
	1,1,1,1,
	1,2,2,1,
	2,0,0,1,
	1,2,2,2,
	2,0,0
};

const int TSSTDData[]=
{
      0,
	 85, 67, 98, 98,
	 85,148,100,104,
	 75,  0,  0,100,
	130,100,150,153,
	150,  0,  0,
};

extern void robot_action_ts15_alllock(int lock);

int robot_action_posi = ROBOT_ACTION_POSI_STAND;
static int last_advance_flag = 0;
static int robot_action_move_step = 0;


int robot_action_remote_keep = 0;
int robot_action_remote_dir = 0;

//uint16_t dt = 0;
uint16_t raw_flag=0;

extern uint8_t adata[200][20];
extern uint16_t aid;
uint16_t R12_ADJ_TSDATA[20]=
{
    100,100,100,100,
    100,100,100,100,
    100,100,100,100,
    100,100,100,100,
    100,100,100,100,
};
int adjoffset = 127;

int ACTION_MODE = ACTION_ALL;

int SLEEP_TIME_ADD = 0;
int ACTION_TIME_LV = 0;

#define GET_TS15_TIME(time) (time*(40+ACTION_TIME_LV))
#define GET_SLEEP_TIME(time) (time*15)

#define TS15_NUM 8

const uint16_t tsidlist[TS15_NUM+1]=
{
#if 0//D05
		0,
		1,2,3,
		4,5,6,
		7,8,9,0,0,
		10,11,12,0,0,
		13
#else//R11,R12
        0,
        1,2,3,
        9,10,11,
        4,5,6,7,8,
        12,13,14,15,16,
        17,
#endif
};

//#define GET_TS15_ID(id)	(id)
#define GET_TS15_ID(id)	(ts15_servo_idx[id])
#define GET_TS15_TIME(time) (time*(40+ACTION_TIME_LV))
#define GET_SLEEP_TIME(time) (time*15)





int16_t R11_ActionBreak_Flag = 0;
int action_shield_flag = 0;//不可打断标志，屏蔽中断信号，用于倒下，起来等不可打断动作

int16_t Action_Keep_Flag = 0;

int16_t TS15_Comm_Test_RunFlag = 0;
int16_t TS15_CommTest_Time;
int16_t TS15_CommTest_OKTime;
int16_t TS15_CommTest_ErrTime;
int16_t TS15_CommTest_OKFlag;

int GET_TS15_POS(int id,int ang)
{
    //if((id==1)||(id==9))
    //    return (((ang+12)*4096)/(264));
    //else
    return (((ang+15)*4096)/(330));
}


int GET_TS15_POS2(int id,int ang)
{
    return (((ang+15)*4096)/(330));
}

int GET_TS15_POS3(int id,int ang)//10为一度
{
    //if((id==1)||(id==9))
    //    return (((ang+12)*4096)/(264));
    //else
    //return (((ang+150)*4096)/(3300));
    return ang;
}


int GET_TS15_ANG(int id,int pos)
{
    if((id==1)||(id==9))
        return ((pos)*264)/4096-12;
    else
        return (((pos)*330)/4096)-15;
}


const int R12_STD_TSDATA[]=
{
		0,
		2607,	931,	2048,
		1489,	3165,	2048,
		1489,	1738,	2855,	2048,	2048,
		2607,	2358,	1241,	2048,	2048,
		2048,
};


void robot_action_adjusting_save()
{
    int i, page;
    for (i = 0; i < 19; i++)
    {
       adata[0][i] = R12_ADJ_TSDATA[i];
       //DBG_PRINT("adata 1 ", adata[0][i]);
    }
    adata[0][19]=robot_data_checksum_flash(adata[0],19);
    aid = 1;
    //DBG_PRINT("adata[0][19] = ",adata[0][19]);
    page = robot_storage_offset2page(adjoffset);
    DBG_PRINT("robot_action_data_save page = ",page);
    robot_storage_adata_write(page,0);
}


void robot_action_adjusting_load()
{
    int i;
    //aid = 1;
    int adjpage = adjoffset+200;
    aid = robot_storage_adata_read(adjpage);
    if(aid)
    {
        for(i=0;i<20;i++)
        {
            R12_ADJ_TSDATA[i] = adata[0][i];
        }
    }
    else
    {
        for(i=0;i<20;i++)
        {
            R12_ADJ_TSDATA[i] = 100;
        }
    }
}

void TS15_IDMoveSend(uint16_t id,uint16_t move,uint16_t time)
{
		uint8_t para[6];
		uint8_t comm[12];
		uint16_t csz;
		id = GET_TS15_ID(id);
		move = GET_TS15_POS(0,move);
		time = time;
		para[0] = move>>8;
		para[1] = move&0xFF;
		para[2] = time>>8;
		para[3] = time&0xFF;
		csz = robot_bus_command_make(id,0x03,0x2a,4,para,comm);
		BUS_WRITE(comm,csz);
}

int robot_ts01_bus_move_command_make(int idx,int move,int time,uint8_t output[])
{
	int i,k=0;
	unsigned char  cs = 0;
	unsigned short moveu16 = move*20;
	output[k++] = 0xAF;//0 head1
	output[k++] = 0xFA;//1 head2
	output[k++] = idx;//2 id
	output[k++] = 0x0A;
	output[k++] = 0x03;
    output[k++] = 0x00;
    output[k++] = time;
    output[k++] = (moveu16>>8);
    output[k++] = (moveu16&0xff);
	for(i=2;i<k;i++)
		cs += output[i];
	output[k++] = cs;
	return k;
}

int robot_ts01_bus_read_command_make(int idx,uint8_t output[])
{
	int i,k=0;
	unsigned char  cs = 0;
	output[k++] = 0xAF;//0 head1
	output[k++] = 0xFA;//1 head2
	output[k++] = idx;//2 id
	output[k++] = 0x0A;
	output[k++] = 0x25;
    output[k++] = 0x00;
    output[k++] = 0x00;
    output[k++] = 0x00;
    output[k++] = 0x00;
	for(i=2;i<k;i++)
		cs += output[i];
	output[k++] = cs;
	return k;
}


int TS01_IDPosiRead(int id,int raw_flag)
{

   	uint8_t para[6];
    uint8_t comm[12];
    uint8_t csz;
    uint16_t ret = 0;

    para[0] = 2;
    csz = robot_ts01_bus_read_command_make(id,comm);
    BUS_FLUSH();
    robot_bus_read_flag_set(BUS_RF_TS02_POSI,2,20);
    BUS_WRITE(comm,csz);
    ROBOT_DELAY_MS(2);
    while(BUS_READING)//UART_INPUT_HANDLEÄÚ´øÓÐ³¬Ê±ÅÐ¶Ï
    {
        robot_bus_data_read_once();
        ROBOT_DELAY_MS(2);
    }
    if(bus_readid!=id)
        bus_readdata = 0;
    ret = bus_readdata;
    if(!raw_flag)
        ret = (ret+10)/20;
        //ret = GET_TS15_ANG(ts15_id,ret);
    return ret;
}

int TS01_IDSMoveRead(int idst,int ided,int *output)
{
    int i;
    int sz = ided-idst+1;
    for(i=0;i<sz;i++)
    {
        int read = TS01_IDPosiRead(idst+i,0);
        DBG2_PRINT("Read >>",idst,read);
        HEX_PRINT("hex = ",read);
        if(output)
            output[i] = read;
    }
    return sz;
}

void TS01_IDMoveSend(int id,int move,int time)
{
		uint8_t comm[12];
		int csz;
		csz = robot_ts01_bus_move_command_make(id,move,time,comm);
		BUS_WRITE(comm,csz);
}

void TS01_ID2MoveSend(int id1,int id2,int move1,int move2,int time)
{
    int csz = 0;
    int k;
    uint8_t comm[128];
    //DBG2_PRINT("TS01 ",id1,move1);
    //DBG2_PRINT("TS01 ",id2,move2);
    csz = 0;
    k = robot_ts01_bus_move_command_make(id1,move1,time,comm+csz);
    csz+=k;
    k = robot_ts01_bus_move_command_make(id2,move2,time,comm+csz);
    csz+=k;
    BUS_WRITE(comm,csz);
}

void TS01_IDSMoveSend(int idst,int ided,uint16_t moves[],uint16_t time)
{
		uint8_t comm[128];
		int csz = 0;
		int i,j,k;
		j = 0;
		for(i=idst;i<=ided;i++)
        {
            k = robot_ts01_bus_move_command_make(i,moves[j],time,comm+csz);
            j++;
            csz+=k;
        }
		BUS_WRITE(comm,csz);
}

void TS01_IDSMoveSend2(int idst,int ided,uint8_t moves[],uint16_t time)
{
    uint16_t moves16[20];
    int sz = ided-idst+1;
    int i,j;
    //DBG2_PRINT("moves2 = ",moves[2],moves[3]);
    for(i=0;i<sz;i++)
    {
        j = (i<9)?(0):(2);
        moves16[i] = moves[i+j+2];//这有问题，不能兼容部分舵机，只能全部舵机有效。
    }
    TS01_IDSMoveSend(idst,ided,moves16,time);
}

void TS01_IDSMoveSend3(int idst,int ided,uint16_t move,uint16_t time)
{
		uint8_t comm[128];
		int csz = 0;
		int i,j,k;
		j = 0;
		for(i=idst;i<=ided;i++)
        {
            k = robot_ts01_bus_move_command_make(i,move,time,comm+csz);
            j++;
            csz+=k;
        }
		BUS_WRITE(comm,csz);
}

void TS01_IDSMoveSend4(uint16_t ids[],uint16_t moves[],int idsz,uint16_t time)
{
		uint8_t comm[128];
		int csz = 0;
		int i,j,k;
		for(i=0;i<idsz;i++)
        {
            k = robot_ts01_bus_move_command_make(ids[i],moves[i],time,comm+csz);
            csz+=k;
        }
		BUS_WRITE(comm,csz);
}


void TS15_IDTimeSend(int idx,uint16_t time)
{
    uint8_t para[64];
    uint8_t comm[64];
    uint16_t csz;
    int j = 0;
    para[j++]=0x02;
    idx = GET_TS15_ID(idx);
    para[j++] = idx;
    para[j++] = time>>8;
    para[j++] = time&0xFF;
    csz = robot_bus_command_make(0xfe,0x83,0x2c,j,para,comm);
    BUS_WRITE(comm,csz);
    ROBOT_DELAY_MS(2);
    DBG_PRINTF("TS15_IDTimeSend %d %d",idx,time);
}

void TS15_IDSTimeSend(int time)
{
		uint8_t para[2];
		uint8_t comm[20];
		uint16_t csz;
		uint16_t time16 = time;

		int j=0;
		time16 = time;
		DBG_PRINTF("TS15(All) Set Time = %d",time16);
		para[j++] = time16>>8;
		para[j++] = time16&0xFF;
		csz = robot_bus_command_make(0xfe,0x03,0x2c,j,para,comm);
		BUS_WRITE(comm,csz);

}

#if 0
void TS15_IDSPosiSend_asyn(uint8_t moves[])
{
		uint16_t id,move;
		uint8_t para[128];
		uint8_t comm[128];
		uint16_t csz;
		int FBADJ=0;
		int i,j=0;
		para[j++] = 2;
		for(i=0;i<TS15_NUM;i++)
		{
				id = GET_TS15_ID(i+1);
				if(!id)
                    continue;
                move = moves[i];//提前转uint16，避免出现丢数据位。
//				move = GET_TS15_POS(id,move)+R12_ADJ_TSDATA[i+1]-100;
                move = GET_TS15_POS(id,move);
                para[j++] = id;
                para[j++] = move>>8;
                para[j++] = move&0xFF;
		}
		csz = robot_bus_command_make(0xfe,0x04,0x2a,j,para,comm);
		BUS_WRITE(comm,csz);
}
#endif

void TS15_IDSPosiSend_asyn3(uint8_t moves[],uint8_t times[],int idst,int ided)
{
		uint16_t id,move,mtime;
		uint8_t para[128];
		uint8_t comm[128];
		uint16_t csz;
		int FBADJ=0;
		int i,j=0;
		int k=0;
		para[j++] = 4;
		for(i=idst;i<=ided;i++)
		{
				id=i;
				//if(!id)
                //    continue;
                move = moves[k];//提前转uint16，避免出现丢数据位。
                mtime = times[k];
                k++;
                move = GET_TS15_POS(0,move);

                para[j++] = id;
                para[j++] = move>>8;
                para[j++] = move&0xFF;
                para[j++] = mtime>>8;
                para[j++] = mtime&0xFF;
		}
		csz = robot_bus_command_make(0xfe,0x04,0x2a,j,para,comm);
		BUS_WRITE(comm,csz);
}

void TS15_IDSPosiSend_asyn4(uint8_t moves[],uint8_t timein,int idst,int ided)
{
		uint16_t id,move,mtime;
		uint8_t para[128];
		uint8_t comm[128];
		uint16_t csz;
		int FBADJ=0;
		int i,j=0;
		int k=0;
		para[j++] = 4;
		for(i=idst;i<=ided;i++)
		{
				id=i;
				//if(!id)
                //    continue;
                move = moves[k];//提前转uint16，避免出现丢数据位。
                mtime = timein;
                k++;
                move = GET_TS15_POS(0,move);

                para[j++] = id;
                para[j++] = move>>8;
                para[j++] = move&0xFF;
                para[j++] = mtime>>8;
                para[j++] = mtime&0xFF;
		}
		csz = robot_bus_command_make(0xfe,0x04,0x2a,j,para,comm);
		BUS_WRITE(comm,csz);
}

#define RSEND_TIME 5
int robot_action_debug_flag = 1;
void TS15_IDSPosiSend_asyn5(uint8_t moves[])
{
    int i,j,k,s;
    int trytime = RSEND_TIME;
    uint16_t data[20][2];
    uint16_t id,move;

    uint8_t para[128];
    uint8_t comm[128];
    uint16_t csz;

    for(i=0;i<TS15_NUM;i++)
    {
        id = GET_TS15_ID(i+1);
        move = moves[i];
        data[id][0] = 0;
        data[id][1] = GET_TS15_POS(id,move);
        DBG2_PRINT("id & move = ",id,data[id][1]);
        //data[id][1] = GET_TS15_POS(id,move)+R12_ADJ_TSDATA[i+1]-100;
    }
    do
    {
        if(trytime<=0)
            break;
        j = 0;
        s = 0;
        para[j++] = 2;

        uint8_t sent_ids[TS15_NUM];
        int sent_count = 0;

        for(i=1;i<=TS15_NUM;i++)
        {
            if(data[i][0]==0)
            {
                para[j++] = i;
                para[j++] = data[i][1]>>8;
                para[j++] = data[i][1]&0xFF;
                sent_ids[sent_count++] = i; // 记录当前帧发送的ID
                s++;
            }
        }
        //DBG_PRINT("bus send >>",s);

        csz = robot_bus_command_make(0xfe,0x04,0x2a,j,para,comm);
        BUS_WRITE(comm,csz);
        ROBOT_DELAY_MS(5);
        k = BUS_TS15ACK_READ(data,s,40);

        if(robot_action_debug_flag)
            DBG2_PRINT("BUS_S&R = ",s,k);
        int current_attempt = RSEND_TIME - trytime + 1;

        if(robot_action_debug_flag)
        {
            if(k != s)
            {
                DBG_PRINT("Resend Attempt:", current_attempt);
                for(int idx=0; idx<sent_count; idx++)
                {
                    uint8_t current_id = sent_ids[idx];
                    if(data[current_id][0] == 0)
                    {
                        DBG_PRINT("Miss ID in Frame:", current_id);
                    // 若需要位置信息，可添加：DBG_PRINT("Position:", data[current_id][1]);
                    }
                }
            }
        }
        trytime--;
    }while(k!=s);

    if(trytime<=0)
    {
        DBG_PRINT("TRY FAIL NG = ",s-k);
        for(i=1;i<=TS15_NUM;i++)
        {
            if(data[i][0]==0)
                DBG_PRINT("Final Miss ID:",i);
        }
    }
    else
    {
        //if(trytime!=RSEND_TIME-1)
        //    DBG_PRINT("TRY OK RESEND = ",RSEND_TIME-trytime);
    }
}

void TS15_IDSPosiSend_asyn6(int ids[],int moves[],int size)
{
    int i,j;
    uint16_t id,move;

    uint8_t para[128];
    uint8_t comm[128];
    uint16_t csz;

    j = 0;
    para[j++]=0x02;
    for(i=0;i<size;i++)
    {
        id = ids[i];
        move = GET_TS15_POS(id,moves[i]);
        para[j++] = id;
        para[j++] = move>>8;
        para[j++] = move&0xFF;

    }
    csz = robot_bus_command_make(0xfe,0x04,0x2a,j,para,comm);
    BUS_WRITE(comm,csz);
    ROBOT_DELAY_MS(5);
}

void TS15_IDSPosiSend_asyn7(int ids[],int moves[],int time,int size)
{
    int i,j;
    uint16_t id,move;

    uint8_t para[128];
    uint8_t comm[128];
    uint16_t csz;
    uint16_t time16 = time;
    uint16_t timeneck = 240;
    j = 0;
    para[j++]=0x04;
    for(i=0;i<size;i++)
    {
        id = ids[i];
        move = GET_TS15_POS(id,moves[i]);
        para[j++] = id;
        para[j++] = move>>8;
        para[j++] = move&0xFF;
        if(id==1)
        {
            para[j++] = time16>>8;
            para[j++] = time16&0xFF;
        }
        else
        {
            para[j++] = time16>>8;
            para[j++] = time16&0xFF;
        }

    }
    csz = robot_bus_command_make(0xfe,0x04,0x2a,j,para,comm);
    BUS_WRITE(comm,csz);
    ROBOT_DELAY_MS(5);
}




void TS15_asynMove_Set()
{
		uint8_t comm[12];
		uint16_t csz;
		csz = robot_bus_command_make(0xfe,0x05,-1,0,NULL,comm);
		BUS_WRITE(comm,csz);
}

void TS15_IDSPosiSend_syn(int ids[],int moves[],int times[],int size)
{
    int i,j;
    uint16_t id,move,time;

    uint8_t para[128];
    uint8_t comm[128];
    uint16_t csz;
    j = 0;
    para[j++]=0x04;
    for(i=0;i<size;i++)
    {
        id = GET_TS15_ID(ids[i]);
        move = GET_TS15_POS(id,moves[i]);
        time = GET_TS15_TIME(times[i]);
        para[j++] = id;
        para[j++] = move>>8;
        para[j++] = move&0xFF;
        para[j++] = time>>8;
        para[j++] = time&0xFF;
    }
    csz = robot_bus_command_make(0xfe,0x83,0x2a,j,para,comm);
    BUS_WRITE(comm,csz);
    ROBOT_DELAY_MS(2);
}

void TS15_IDSPosiSend_syn2(int ids[],int moves[],int size)
{
    int i,j;
    uint16_t id,move,time;

    uint8_t para[64];
    uint8_t comm[64];
    uint16_t csz;
    j = 0;
    para[j++]=0x02;
    for(i=0;i<size;i++)
    {
        id = GET_TS15_ID(ids[i]);
        move = GET_TS15_POS3(id,moves[i]);
        para[j++] = id;
        para[j++] = move>>8;
        para[j++] = move&0xFF;
    }
    csz = robot_bus_command_make(0xfe,0x83,0x2a,j,para,comm);
    //DBG_PRINT("bus send",0);
    BUS_WRITE(comm,csz);
    //ROBOT_DELAY_MS(2);
}

void TS15_IDSPosiSend_syn3(int ids[],int moves[],int times[],int size)
{
    int i,j;
    uint16_t id,move,time;

    uint8_t para[96];
    uint8_t comm[96];
    uint16_t csz;
    j = 0;
    para[j++]=0x02;
    for(i=0;i<size;i++)
    {
        id = GET_TS15_ID(ids[i]);
        move = moves[i];
        time = times[i];
        para[j++] = id;
        para[j++] = move>>8;
        para[j++] = move&0xFF;
        para[j++] = time>>8;
        para[j++] = time&0xFF;
    }
    csz = robot_bus_command_make(0xfe,0x83,0x2a,j,para,comm);
    BUS_WRITE(comm,csz);
}

void TS15_IDSPosiSend_syn4(int ids[], int moves[], int times[], int size)
{
    // 计算所需缓冲区大小：头2 + ID1 + 长度1 + 命令1 + 地址1 + 参数(1+5*size) + 校验1
    //int total_size = 2 + 1 + 1 + 1 + 1 + (1 + 5 * size) + 1;
    uint8_t comm[80]; // 确保足够大
    unsigned char cs = 0;
    int k = 0;

    // 构建指令头
    comm[k++] = 0xFF; // head1
    comm[k++] = 0xFF; // head2
    comm[k++] = 0xFE; // ID

    // 长度字段：命令1 + 地址1 + 参数长度(1+5*size) + CS
    int length_field = 1 + 1 + (1 + 5 * size) + 1;
    comm[k++] = length_field;

    // 命令和地址
    comm[k++] = 0x83; // command
    comm[k++] = 0x2A; // address

    // 参数部分
    comm[k++] = 0x04; // parameter header

    // 直接填充舵机数据
    for(int i = 0; i < size; i++) {
        uint16_t id = GET_TS15_ID(ids[i]);
        uint16_t move = moves[i];
        uint16_t time = times[i];

        comm[k++] = id;
        comm[k++] = move >> 8;
        comm[k++] = move & 0xFF;
        comm[k++] = time >> 8;
        comm[k++] = time & 0xFF;
    }

    // 计算校验和（从ID字段开始，索引2到k-1）
    for(int j = 2; j < k; j++) {
        cs += comm[j];
    }
    cs ^= 0xFF;
    if(cs == 0xFF) {
        cs = 0x00;
    }
    comm[k++] = cs;

    BUS_WRITE(comm, k);
}

#define TS15CTL_MAXSIZE 20
#define TS15CTL_TSNUM 10
int ts15ctl_ids[TS15CTL_MAXSIZE];
int ts15ctl_posi[TS15CTL_MAXSIZE];
int ts15ctl_time[TS15CTL_TSNUM+1];
int ts15ctl_size = 0;
int ts15ctl_debug = 0;
int ts15ctl_send = 1;
int ts15ctl_debugidx = 0;//0全部显示 1-9，只显示对应ID
int ts15ctl_makeflag = 0;
/*
void ts15ctl_settime(int time)
{
    ts15ctl_time = time;
    ts15ctl_timemode = 0;
}
*/

void ts15ctl_debug_set(int debug)
{
    ts15ctl_debug = debug;
}

void ts15ctl_settimes(int time)
{
    for(int i=1;i<=TS15CTL_TSNUM;i++)
        ts15ctl_time[i] = time;
}

void ts15ctl_settime(int idx,int time)
{
    ts15ctl_time[idx+1] = time;
}

void ts15ctl_clear()
{
    ts15ctl_size = 0;
}

void ts15ctl_add(int id,int posi)
{
    if(ts15ctl_size<TS15CTL_MAXSIZE)
    {
        ts15ctl_ids[ts15ctl_size]=id;
        ts15ctl_posi[ts15ctl_size]=ts15_servo_base[id]+posi*ts15_servo_dir[id];
        ts15ctl_size++;
    }
}



void ts15ctl_make()
{
    //if(ts15ctl_debug)
    //    DBG_PRINT("ts15ctl_size = ",ts15ctl_size);
    int i;
    int ids[20];
    int moves[20];
    int times[20];
    if(!ts15ctl_size)
        return ;
    if(ts15ctl_debug)
    {
        static int ts15ctl_debug_idx = 0;
        int i;
        DBG_PRINT("ts15make >>",ts15ctl_debug_idx++);
        for(i=0;i<ts15ctl_size;i++)
        {
            if((ts15ctl_debugidx==0)||(ts15ctl_debugidx==ts15ctl_ids[i]))
                DBG2_PRINT("ts15data >> ",ts15ctl_ids[i],ts15ctl_posi[i]);
        }
    }
#if 0
    if(ts15ctl_timemode==0)
    {
        TS15_IDSTimeSend(ts15ctl_time);
        ROBOT_DELAY_MS(4);
    }
    //TS15_IDSPosiSend_asyn6(ts15ctl_ids,ts15ctl_posi,ts15ctl_size);
    TS15_IDSPosiSend_asyn7(ts15ctl_ids,ts15ctl_posi,ts15ctl_time,ts15ctl_size);
    ROBOT_DELAY_MS(20);
    TS15_asynMove_Set();
#else
    if(ts15ctl_send)
    {
        for(i=0;i<ts15ctl_size;i++)
        {
            ids[i] = ts15ctl_ids[i];
            moves[i] = ts15ctl_posi[i];
            times[i] = ts15ctl_time[ids[i]];
            //times[i] = ts15ctl_time;
        }
        ts15ctl_makeflag = 1;
        TS15_IDSPosiSend_syn4(ids,moves,times,ts15ctl_size);
        ts15ctl_makeflag = 0;
    }

    //TS15_IDSPosiSend_syn(ids,moves,times,ts15ctl_size);
    //ROBOT_DELAY_MS(4);
#endif
}

void ts15ctl_debug_do()
{
    int i;
    ts15ctl_clear();
    for(i=0;i<8;i++)
    {
        ts15ctl_add(i,0);
    }
    //DBG_PRINT("ts15make",0);
    ts15ctl_make();
    //DBG_PRINT("ts15make done",0);
}



int TS15_IDPosiRead(int id,int raw_flag)
{
   	uint8_t para[6];
    uint8_t comm[12];
    uint8_t csz;
    uint16_t ret = 0;

    int ts15_id = GET_TS15_ID(id);
    DBG_PRINT("TS15_IDPosiRead",ts15_id);
    para[0] = 2;
    csz = robot_bus_command_make(ts15_id,0x02,0x38,1,para,comm);
    BUS_FLUSH();
    robot_bus_read_flag_set(BUS_RF_TS15_POSI,2,20);
    BUS_WRITE(comm,csz);
    BUS_SETIN();
    ROBOT_DELAY_MS(2);
    while(BUS_READING)//UART_INPUT_HANDLEÄÚ´øÓÐ³¬Ê±ÅÐ¶Ï
    {
        robot_bus_data_read_once();
        ROBOT_DELAY_MS(2);
    }
    BUS_SETOUT();
    if(bus_readid!=ts15_id)
        bus_readdata = 0;
    ret = bus_readdata;
    if(!raw_flag)
        ret = (bus_readdata+10)/20;

    return ret;
}

int robot_bus_ts15_idle()
{
    int readflag,reading;
    readflag = bus_readflag;
    reading = BUS_READING;
    return !reading;
}



void TS15_IDLock_Send(int id,int lock)
{
		uint8_t para[6];
		uint8_t comm[12];
		uint8_t csz;

		para[0] = lock;
		csz = robot_bus_command_make(GET_TS15_ID(id),0x03,0x28,1,para,comm);
		BUS_WRITE(comm,csz);
		ROBOT_DELAY_MS(5);
}

void robot_action_ts15_lock()
{
    uint16_t j;
    STR_PRINT("ts15 lock");
    for(j=1;j<=TS15_NUM;j++)
    {
        if(adata[0][j+1]==1)
            TS15_IDLock_Send(j,1);
        else if(adata[0][j+1]==2)
            TS15_IDLock_Send(j,0);

    }
}



void robot_action_ts15_alllock(int lock)
{
    uint16_t j;
    for(j=1;j<TS15_NUM;j++)
    {
        TS15_IDLock_Send(j,lock);
    }
    STR_PRINT("ts15 lock");
}


void TS15_Comm_Test_Flag_Set(int flag)
{
    TS15_Comm_Test_RunFlag = flag;
}

void TS15_COMM_Test_Once()
{
    if(TS15_Comm_Test_RunFlag)
    {
        if(!TS15_IDSBackCodeGet())
        {
            TS15_CommTest_OKFlag = 0;
            TS15_CommTest_ErrTime++;
        }
    }
    else
    {
        #if TS15_TEST_MODE
        TS15_IDSBackCodeGet();
        #endif
        ROBOT_DELAY_MS(25);
    }
}

int robot_action_get_show_id(uint8_t data[])
{
    #if 1
    int i;
    int show_id = data[0];

    for(i=1;i<ACTION_DATA_SZ-1;i++)
    {
        if(data[i]!=0)
        {
            show_id = 0;
            break;
        }
    }
    if(show_id)
        DBG_PRINT("robot_action_get_show_id show_id = ",show_id);
    return show_id;
    #else
    return 0;
    #endif
}



void robot_audio_voice_play(uint16_t audio_id)
{
    #if 1
    char fname[64];
    sprintf(fname,"voice%d.mp3",audio_id);
    DBG_PRINT("robot_action_audio_play idx = ",audio_id);
    robot_audio_play_break();
    os_set_play_audio(fname,NULL);
    #endif
}

void robot_audio_music_play(uint16_t music_id)
{
    #if 0
    char fname[64];
    if(music_id == 0)
    {
        music_id = robot_action_play_idx_get()%10+1;
    }
    sprintf(fname,"music%d.mp3",music_id);
    DBG_PRINT("robot_action_music_play idx = ",music_id);
    robot_audio_play_break();
    os_set_play_audio(fname,NULL);
    #endif
}

void robot_audio_tone_play(uint16_t type,uint16_t tone)
{
    #if 0
    char fname[64];
    sprintf(fname,"tone_%d_%d.mp3",type,tone);
    DBG_PRINT("robot_action_tone_play type = ",type);
    DBG_PRINT("robot_action_tone_play tone = ",tone);
    robot_audio_play_break();
    os_set_play_audio(fname,NULL);
    #endif
}

void robot_audio_play_break()
{
    #if 1
    if(os_get_play_status())
    {
        os_set_play_break();
        ROBOT_DELAY_MS(20);
    }

    #endif
}

void robot_audio_wait_stop()
{
    //int timer = 0;
    #if 1
    while(1)
    {
        DBG_PRINT("DAC = ",os_get_play_status());
        if(!os_get_play_status())
            break;
    }
    #endif
    //DBG_PRINT()
}


#define ROBOT_LCD_DEV_ID 33

//lcd cmd
#define ROBOT_LCD_MODE_CMD 1
#define ROBOT_LCD_WRITE_CMD 2
#define ROBOT_LCD_READ_CMD 3
#define ROBOT_LCD_CONTROL_CMD 4

//mode
#define ROBOT_LCD_MODE_EMOJI 0x50
#define ROBOT_LCD_MODE_CHAR  0x51

//write addr
#define ROBOT_LCD_EMOJI_ADDR 0x10
#define ROBOT_LCD_CHAR_ASCII_ADDR 0x11
#define ROBOT_LCD_CHAR_BCOLOR_ADDR 0x12
#define ROBOT_LCD_CHAR_CCOLOR_ADDR 0x13
//#define ROBOT_LCD_CHAR_NUM_ADDR 0x12

//control
#define ROBOT_LCD_CONTROL_EMOJI_PLAY 0x21
#define ROBOT_LCD_CONTROL_EMOJI_PLAUSE 0x22
#define ROBOT_LCD_CONTROL_EMOJI_SPEED_SET 0x30
#define ROBOT_LCD_CONTROL_CHAR_CLR 0x40

void robot_lcd_bus_data_send(uint16_t cmd,uint16_t addr,uint8_t *para,uint16_t psz)
{
    #if 0
    uint8_t comm[32];
    uint16_t csz;
    csz = robot_bus_command_make(ROBOT_LCD_DEV_ID,cmd,addr,psz,para,comm);
    ROBOT_DELAY_MS(10);
    BUS_WRITE(comm,csz);
    ROBOT_DELAY_MS(10);
    #endif
}

void robot_lcd_mode_set(uint16_t mode)
{
    #if 0
    DBG_PRINT("robot_lcd_mode_set mode = ",mode);
    if(mode == 1)
        robot_lcd_bus_data_send(ROBOT_LCD_MODE_CMD,ROBOT_LCD_MODE_EMOJI,NULL,0);
    else if(mode == 2)
        robot_lcd_bus_data_send(ROBOT_LCD_MODE_CMD,ROBOT_LCD_MODE_CHAR,NULL,0);
    #endif
}

uint16_t robot_lcd_get_good_eat()
{
    #if 0
    const uint16_t goog_list[5] = {1,2,4,7,9};
    int r = robot_action_play_idx_get2();
    return goog_list[r%5]+60;
    #else
    return 0;
    #endif
}

uint16_t robot_lcd_get_bad_eat()
{
    #if 0
    const uint16_t bad_list[5] = {10,13,14,17,18};
    int r = robot_action_play_idx_get2();
    return bad_list[r%5]+60;
    #else
    return 0;
    #endif
}

void robot_lcd_emoji_set(uint16_t emoji_id)
{
    #if 0
    uint8_t para[2];
    if(emoji_id==91)
        emoji_id = robot_lcd_get_good_eat();
    else if(emoji_id==92)
        emoji_id = robot_lcd_get_bad_eat();
    para[0] = emoji_id;
    DBG_PRINT("robot_action_lcd_emoji_set idx = ",emoji_id);

    robot_lcd_bus_data_send(ROBOT_LCD_WRITE_CMD,ROBOT_LCD_EMOJI_ADDR,para,1);
    #endif
}

void robot_lcd_emoji_read()
{
    #if 0
    DBG_PRINT("robot_action_lcd_emoji_read ",1);
    robot_lcd_bus_data_send(ROBOT_LCD_READ_CMD,ROBOT_LCD_EMOJI_ADDR,NULL,0);
    #endif
}

void robot_lcd_emoji_play_playpause(uint16_t playplause)
{
    #if 0
    DBG_PRINT("robot_action_emoji_play_playpause idx = ",playplause);
    robot_lcd_bus_data_send(ROBOT_LCD_CONTROL_CMD,ROBOT_LCD_CONTROL_EMOJI_PLAY+playplause-1,NULL,0);
    #endif
}

void robot_lcd_emoji_set_speed(uint16_t speed)
{
    #if 0
    DBG_PRINT("robot_action_emoji_set_speed idx = ",speed);
    robot_lcd_bus_data_send(ROBOT_LCD_CONTROL_CMD,ROBOT_LCD_CONTROL_EMOJI_SPEED_SET+speed,NULL,0);
    #endif
}

void robot_lcd_char_set_ascii(int8_t ascii,uint8_t eyeid,uint8_t x,uint8_t y)
{
    #if 0
    uint8_t para[4];
    para[0] = eyeid;
    para[1] = x;
    para[2] = y;
    para[3] = ascii;
    DBG_PRINT("robot_lcd_char_set_ascii ascii = ",ascii);
    DBG_PRINT("robot_lcd_char_set_ascii x = ",x);
    DBG_PRINT("robot_lcd_char_set_ascii x = ",y);
    robot_lcd_bus_data_send(ROBOT_LCD_WRITE_CMD,ROBOT_LCD_CHAR_ASCII_ADDR,para,4);
    #endif
}

void robot_lcd_char_set_asciis(int8_t asciis[],int16_t sz,uint8_t eyeid,uint8_t x,uint8_t y)
{
    #if 0
    int i;
    for(i=0;i<sz;i++)
    {
        robot_lcd_char_set_ascii(asciis[i],eyeid,x+i,y);
    }
    #endif
}
//mode 两种模式，0：正常显示数字，有多少位显示多少位，1，显示length位，如果不够，前补0
uint16_t robot_lcd_char_set_num_int(int16_t num_i,uint8_t eyeid,uint16_t x,uint16_t y,uint16_t length,int mode)
{
    #if 0
    char str[12];
    int slen = 0;
    int temp = num_i;
    int temp2 = length;
    int i,k = x;
    if(num_i<0)
    {
        num_i = -num_i;
        str[slen++]='-';
    }
    while(1)
    {
        if(mode==0)
        {
            if(!temp)
                break;
        }
        else
        {
            if( (temp2--) < 0)
                break;
        }
        str[slen++] = temp%10+'0';
        temp/=10;
    }
    if(slen==0)
        str[slen++] = '0';
    if(slen>length)
        length = slen;
    for(i=slen-1;i>=0;i--)
        robot_lcd_char_set_ascii(str[i],eyeid,k++,y);
    for(i=0;i<length-slen;i++)
        robot_lcd_char_set_ascii(' ',eyeid,k++,y);
    return length;
    #else
    return 0;
    #endif
}

uint16_t robot_lcd_char_set_num_double(double num_f,uint8_t eyeid,uint16_t x,uint16_t y,uint16_t length)
{
    #if 0
    int a = (int)num_f;
    int b = (int)((num_f-a)*100+0.5);
    int i,k = x;
    int length1;
    k += robot_lcd_char_set_num_int(a,eyeid,k,y,0,0);
    robot_lcd_char_set_ascii('.',eyeid,k++,y);
    k += robot_lcd_char_set_num_int(b,eyeid,k,y,2,1);
    while(k<length)
        robot_lcd_char_set_ascii(' ',eyeid,k++,y);
    return k;
    #else
    return 0;
    #endif
}

void robot_lcd_char_set_bcolour(uint8_t eyeid,uint8_t colour)
{
    #if 0
    uint8_t para[2];
    para[0] = eyeid;
    para[1] = colour;
    DBG_PRINT("robot_lcd_char_set_bcolour = ",colour);
    robot_lcd_bus_data_send(ROBOT_LCD_WRITE_CMD,ROBOT_LCD_CHAR_BCOLOR_ADDR,para,2);
    #endif
}

void robot_lcd_char_set_ccolour(uint8_t eyeid,uint8_t colour)
{
    #if 0
    uint8_t para[2];
    para[0] = eyeid;
    para[1] = colour;
    DBG_PRINT("robot_lcd_char_set_ccolour idx = ",colour);
    robot_lcd_bus_data_send(ROBOT_LCD_WRITE_CMD,ROBOT_LCD_CHAR_CCOLOR_ADDR,para,2);
    #endif
}

void robot_lcd_char_set_clr(uint8_t eyeid,uint16_t st,uint16_t ed)
{
    #if 0
    uint8_t para[3];
    para[0] = eyeid;
    para[1] = st;
    para[2] = ed;
    robot_lcd_bus_data_send(ROBOT_LCD_CONTROL_CMD,ROBOT_LCD_CONTROL_CHAR_CLR,para,3);
    #endif
}

#if 0
int robot_action_data_make()
{

    uint16_t i,j;
    int tm1,tm2;
    int ret = 1;
    DBG_PRINT("action send frame_sz = ",aid);
		//if(ACTION_MODE != ACTION_ALL)
		//{
		//		return 1;
		//}

    R11_ActionBreak_Flag = 0;

    for(i=0;i<aid;i++)
    {
        if(action_shield_flag==0)
        {
            if(Action_Keep_Flag)
            {
            }
            else
            {
                if(0)
                {
                    STR_PRINT("action break\r\n");
                    R11_ActionBreak_Flag = 1;
                    ret = 0;
                    break;
                }
            }
        }
        DBG2_PRINT("ActionTag : ",adata[i][0],adata[i][1]);
        if(adata[i][1]==0)//播放音频表情
        {
            int show_id = robot_action_get_show_id(adata[i]);
            if(show_id>0)
            {
                if(show_id == ROBOT_ACTION_SHOW_AUDIO_STOP)
                    robot_audio_play_break();
                if((show_id>=ROBOT_ACTION_SHOW_AUDIO_ST)&&
                    (show_id<ROBOT_ACTION_SHOW_AUDIO_ST+ROBOT_ACTION_SHOW_AUDIO_SZ))
                    robot_audio_voice_play(show_id - ROBOT_ACTION_SHOW_AUDIO_ST + 1);
                if((show_id>=ROBOT_ACTION_SHOW_MUSIC_ST)&&
                    (show_id<ROBOT_ACTION_SHOW_MUSIC_ST+ROBOT_ACTION_SHOW_MUSIC_SZ))
                    robot_audio_music_play(show_id - ROBOT_ACTION_SHOW_MUSIC_ST);
                if((show_id>=ROBOT_ACTION_SHOW_EMOJI_ST)&&
                    (show_id<ROBOT_ACTION_SHOW_EMOJI_ST+ROBOT_ACTION_SHOW_EMOJI_SZ))
                    robot_lcd_emoji_set(show_id - ROBOT_ACTION_SHOW_EMOJI_ST);
                ROBOT_DELAY_MS(20);
                continue;
            }
        }
        if(adata[i][0]==0)
        {
             continue;
        }
#if 1

#else
        TS15_IDSTimeSend(adata[i][0]);
        ROBOT_DELAY_MS(5);
        TS15_IDSPosiSend_asyn(&adata[i][2]);
        ROBOT_DELAY_MS(5);
        TS15_COMM_Test_Once();
        TS15_asynMove_Set();
        ROBOT_DELAY_MS(5);
        if(SLEEP_TIME_ADD)
            ROBOT_DELAY_MS(SLEEP_TIME_ADD);
        for(j=1;j<adata[i][1];j++)
        {
            ROBOT_DELAY_MS(15);
        }
#endif
    }
    aid = 0;//动作做完了，清空aid，避免新的动作会叠加以前的动作
    if(ret)
    {
        STR_PRINT("TS15_ActionSend OK\r\n");
#if 0
				if(R11_LowPowerPlay)
				{
						//AC79_Data_Send(R11_SET,R11_NG_LP,NULL,0);
						R11_LowPowerPlay = 0;
				}
				else
				{
						AC79_Data_Send(R11_SET,R11_OK,NULL,0);
				}
#endif
    }
    return ret;
}
#endif

void robot_action_ts15_posi_send(int rdata[])
{

    uint8_t sbuff[3+TS15_NUM];//2+17+1;
    int i;
    sbuff[0] = 0xAB;
    sbuff[1] = 0xBA;
    for(i=1;i<=TS15_NUM;i++)
        sbuff[i+1] = rdata[i];
    sbuff[TS15_NUM+2] = robot_control_communication_checksum(sbuff,TS15_NUM+2);
    MAIN_SEND(sbuff,TS15_NUM+3);
}

void robot_action_ts15_posi_read(void)
{
    int j;
    int rdata[20];
    for(j=1;j<=TS15_NUM;j++)
    {
     // if(adata[0][j])
      rdata[j] = TS15_IDPosiRead(j,1);
      if(rdata[j]==0)
      {
        ROBOT_DELAY_MS(40);
        rdata[j] = TS15_IDPosiRead(j,1);
      }
      DBG_PRINT("rdata",rdata[j]);
    }
    ROBOT_DELAY_MS(5);
    //robot_action_ts15_posi_send(rdata);
}

typedef struct
{
    int idx;
    char name[32];
    int speaktime;
}audio_info;

audio_info audio_list[]=
{
    {0,"null",0},
    {1,"open",6},
    {2,"netok",9},
    {3,"netdisok",3},
    {4,"netnook",11},
    {5,"btok",4},
    {6,"btdisok",4},
    {7,"appok",4},
    {8,"wakeup",1},
    {9,"bye",5},
};

char action_audio[64];



char* get_action_audio(int type)
{

    int idx = aiui_get_vcn();
    if(idx>8)
        idx = 1;//暂时
    if(idx)
        sprintf(action_audio,"%s%d.mp3",audio_list[type].name,idx);
    return action_audio;
}

void robot_action_audio_play(uint16_t idx)
{
    if(idx)
    {
        //tbz_speak_set_do(audio_list[idx].speaktime);
        tbz_speak_tone_do();
        if(idx<=10)
            os_set_play_audio(get_action_audio(idx),NULL);
        else
        {
            sprintf(action_audio,"touch%d.mp3",idx-10);
            os_set_play_audio(action_audio,NULL);
        }

    }
}

void robot_action_group_play(uint16_t idx)
{
    DBG_PRINT("robot_action_group_play idx = ",idx);
    robot_storage_adata_read(idx);
    //robot_action_data_make();
    robot_action_th01_data_make();
}

void robot_action_system_group_play(uint16_t idx)
{
    robot_action_group_play(ROBOT_STORAGE_SMALL_PAGE_ST+idx);
}



void robot_action_adjusting_compare(int saveflag)
{
    int k,i;
    int errflag = 0;
    for(i=1;i<=TS15_NUM;i++)
    {
        k=TS15_IDPosiRead(i,1);
        if(k==0)
        {
            ROBOT_DELAY_MS(40);
            k=TS15_IDPosiRead(i,1);
        }
    }
}


void robot_action_data_saveplay(uint16_t type,uint16_t offset)
{
    int page = offset;
    DBG2_PRINT("robot_action_data_saveplay page = ",type,offset);
    if(type<=2)
        page = robot_storage_offset2page(offset);
    if(offset<0)
        return ;
    switch(type)
    {
    case 1://save
        robot_storage_adata_write(page,0);
        break;
    case 2://play
        robot_action_group_play(page);
        break;
    case 3://audio
        //robot_action_audio_play(page);
        robot_control_preset_do(offset);
        break;
    }
}

void robot_action_posi_trans(int next_posi)
{
    int action = ROBOT_ACTION_SYSTEM_TRANS_ST+
                    robot_action_posi*ROBOT_ACTION_POSI_SZ +next_posi;
    robot_action_system_group_play(action);
    robot_action_posi = next_posi;
}

void robot_action_advance_play(times)
{
    int i;
    int count_l = 0;
    int count_r = 0;
    int gyroData;
    int baseGyroData;
    int currentGyroData;

    baseGyroData = read_gyroData();
    DBG_PRINT("Base Gyro Data = ", baseGyroData);

    robot_action_move_step = 0;

    for (i = 0; i < times; i++)
    {
            switch(robot_action_move_step)
            {
            case 0:
                robot_action_data_saveplay(2,41);//左起步
                robot_action_move_step = 1;
                break;
            case 1:
                robot_action_data_saveplay(2,42);//右脚前移
                robot_action_move_step = 2;
                break;
            case 2:
                robot_action_data_saveplay(2,43);//左脚前移
                robot_action_move_step = 1;
                break;
            }
    }
    //前进停止，收脚
    if (robot_action_move_step == 1)
        robot_action_data_saveplay(2, 45);//收左脚
    else if (robot_action_move_step == 2)
        robot_action_data_saveplay(2, 44);//收右脚
}

int robot_action_command_advance_remoteflag = 0;
int robot_action_command_advance_hold = 0;

void robot_action_command_advance_hold_set(int hold)
{
    robot_action_command_advance_hold = hold;
}

int robot_action_command_advance_ishold()
{
    return robot_action_command_advance_hold;
}
void robot_action_command_advance_loop(int advance_flag_input) {
    int i;
    int gyroData;
    int baseGyroData;
    int currentGyroData;

    baseGyroData = read_gyroData();
    DBG_PRINT("Base Gyro Data = ", baseGyroData);

    robot_action_move_step = 0;
    robot_action_command_advance_remoteflag = 2;
    robot_action_command_advance_hold_set(1);

    while(1)
    {
        if(!robot_action_command_advance_ishold())
            break;
        robot_action_command_advance_hold_set(0);
        switch(robot_action_move_step)
        {
        case 0:
            robot_action_data_saveplay(2,41);//左起步
            robot_action_move_step = 1;
            break;
        case 1:
            robot_action_data_saveplay(2,42);//右脚前移
            robot_action_move_step = 2;
            break;
        case 2:
            robot_action_data_saveplay(2,43);//左脚前移
            robot_action_move_step = 1;
            break;
        }
    }

    if (robot_action_move_step == 1)
        robot_action_data_saveplay(2, 44);//收左脚
    else if (robot_action_move_step == 2)
        robot_action_data_saveplay(2, 45);//收右脚
    robot_action_command_advance_remoteflag = 0;
}

void robot_action_data_play(int group_idx, int action_idx, int command_type, int times) {
    int tar_posi;
    int play_idx, page_idx;

    // 计算目标姿态位置
//    tar_posi = (group_idx == 0x01) ? ROBOT_ACTION_POSI_STAND : group_idx - 2;

    // 如果姿态不对，先做姿态转换
//    if (tar_posi != robot_action_posi || (action_idx == 0 && command_type == 1)) {
//        robot_action_posi_trans(tar_posi);
//        return;
//    }

    play_idx = calculate_play_index(group_idx, action_idx, command_type, times);

    if (play_idx) {
        robot_action_system_group_play(play_idx);
    }
}

int calculate_play_index(int group_idx, int action_idx, int command_type, int times) {
    int play_idx = 0;
    int page_idx;

    DBG_PRINT("group_idx = ", group_idx);
    DBG_PRINT("action_idx = ", action_idx);
    DBG_PRINT("command_type = ", command_type);

    if (group_idx == 0x01) {
        switch (action_idx) {
            case 1:
                robot_action_system_group_play(action_idx);
                break;
            case 2:
                if (command_type == 1) {
                    robot_action_advance_play(times);
                } else if (command_type == 2) {
                    robot_action_command_advance_loop(times);
                }
                break;
            case 3:
                // robot_action_back_play(times);
                break;
            default:
                play_idx = action_idx;
                break;
        }
    } else {
        page_idx = (group_idx - 1) * 10;
        play_idx = page_idx + action_idx;
    }

    return play_idx;
}


void robot_action_back_play(times)
{
    int i;
    for (i = 0; i < times; i++)
    {
        robot_action_data_saveplay(2,3);
    }
}

int robot_action_app_play_list[18][2]=
{
    {21,1},{22,1},{23,2},{25,2},{27,1},{28,1},
    {51,1},{52,1},{53,1},{54,1},{55,3},{58,2},
    {81,1},{82,1},{83,1},{84,2},{86,1},{87,2},
};
void robot_action_program_play(uint16_t group_idx,uint16_t action_idx,uint16_t times)
{
    robot_action_data_play(group_idx,action_idx,1,times);
}

void robot_action_command_play(uint16_t group_idx,uint16_t action_idx,uint16_t times)
{
    robot_action_data_play(group_idx,action_idx,2,times);
}

void robot_action_groupshow_play(uint16_t group_idx,uint16_t action_idx,uint16_t times)
{
    robot_action_data_play(group_idx,action_idx,3,0);
}

void robot_action_cmd_play(uint16_t cmd,uint16_t times)
{
    int i;
    if(times == 0)
        times = 1;
    if(cmd != 1)
    {
        switch(robot_action_move_step)
        {
        case 1:robot_action_data_play(0x01,0x04,1,0);break;
        case 2:robot_action_data_play(0x01,0x05,1,0);break;
        }
        robot_action_move_step = 0;
    }
    switch(cmd)
    {
    case 1:
        for(i=0;i<times;i++)
        {
            switch(robot_action_move_step)
            {
            case 0:
                robot_action_data_play(0x01,0x01,1,0);
                robot_action_move_step = 1;
                break;
            case 1:
                robot_action_data_play(0x01,0x02,1,0);
                robot_action_move_step = 2;
                break;
            case 2:
                robot_action_data_play(0x01,0x03,1,0);
                robot_action_move_step = 1;
                break;
            }
        }
        break;
    case 2:
    case 3:
    case 4:
        for(i=0;i<times;i++)
        {
            robot_action_data_play(0x01,(cmd-2)+0x06,1,0);
        }
        break;
    default:
        break;
    }
}



int read_gyroData()
{
    uint8_t para[128];
    uint8_t comm[128];
    uint16_t csz;
    int ret;
    STR_PRINT("READ GYRO\r\n");

    para[0] = 0x04;
    csz = robot_bus_command_make(0x4F, 0x02, 0x02, 1, para, comm);
    BUS_FLUSH();
    robot_bus_read_flag_set(BUS_RF_ROBOT_GYRO,4,20);
    BUS_WRITE(comm,csz);
    ROBOT_DELAY_MS(2);
    while(BUS_READING)
    {
        robot_bus_data_read_once();
        ROBOT_DELAY_MS(2);
    }
    ret = bus_readdata;
    return ret;
}

 void robot_program_write_distance(uint16_t mode)
 {
    uint8_t para[128];
    uint8_t comm[128];
    uint16_t csz;
    STR_PRINT("WRITE DISTANCE\r\n");

    switch(mode)
    {
    case 1:
        para[0] = 0x01;
        break;
    case 2:
        para[0] = 0x02;
        break;
    case 3:
        para[0] = 0x03;
        break;
    }
    csz = robot_bus_command_make(0x4F, 0x03, 0x01, 1, para, comm);
    BUS_FLUSH();
    robot_bus_read_flag_set(BUS_RF_ROBOT_GYRO,4,20);
    BUS_WRITE(comm,csz);
 }


void robot_program_distance_make(int16_t cmd,uint8_t para[])
{
    uint16_t mode = para[0];
    switch(cmd)
    {
    case 1:
        robot_program_write_distance(mode);
        break;
    case 2:
        robot_posi_data_once(0,2);
        break;
    }
}


void robot_action_advance_adjustment()
{
    int gyroData;
    switch(gyroData>0)
    {
    case 0:
        robot_action_data_saveplay(2,4);
        break;
    case 1:
        robot_action_data_saveplay(2,4);
        break;
    }
}


void robot_remote_keep_set(uint16_t keep)
{
    robot_action_remote_keep = keep;
    if(keep)
        STR_PRINT("remote keep SET\r\n");
    else
        STR_PRINT("remote keep CLR\r\n");
}

void robot_remote_dir_set(uint16_t dir)
{
    robot_action_remote_dir = dir;
    robot_action_remote_keep = 1;
}

void robot_remote_keep_once()
{
    if(robot_action_remote_keep)
    {
        robot_action_remote_keep = 0;
        switch(robot_action_remote_dir)
        {
            case 1:robot_action_cmd_play(1,1);break;
            case 2:robot_action_cmd_play(2,1);break;
            case 3:robot_action_cmd_play(3,1);break;
            case 4:robot_action_cmd_play(4,1);break;
        }
    }
    else
    {
        if(robot_action_remote_dir==1)
        {
            robot_action_cmd_play(0,0);
        }
        robot_action_remote_dir = 0;
    }
}


void robot_audio_cmd_play(int16_t audio,int16_t wait)
{
    if(wait)
        DBG_PRINT("robot_audio_cmd_play_wait",audio);
    else
        DBG_PRINT("robot_audio_cmd_play_nowait",audio);
    if(audio == 0)
    {
        robot_audio_play_break();
    }
    else if(NUMBER_IN(audio,1,20))
    {
        robot_audio_voice_play(audio);
    }
    else if(NUMBER_IN(audio,21,30))
    {
        robot_audio_music_play(audio-20);
    }
    else if(NUMBER_IN(audio,31,90))
    {
        audio = audio-30;
        robot_audio_tone_play(audio/20+1,audio%20);
    }
    if(wait)
    {
        robot_audio_wait_stop();
    }
}

void robot_emoji_cmd_play(int16_t emoji)
{
    DBG_PRINT("robot_emoji_cmd_play",emoji);
    robot_lcd_emoji_set(emoji);
}

void robot_action_stunt_random_play()
{
    int r1 = RANDOM(3)+2;
    int r2 = RANDOM(6)+1;
    DBG_PRINT("random stunt group =",r1);
    DBG_PRINT("random stunt action =",r2);
    robot_audio_cmd_play(4,1);
    ROBOT_DELAY_MS(1000);
    robot_action_data_play(r1,r2,1,0);
    robot_action_data_play(1,0,2,0);//站回来
}

static uint16_t robot_action_play_add = 0;
static uint16_t robot_action_play_add2 = 0;

uint16_t robot_action_play_idx_get()
{
    uint16_t ret = robot_action_play_add;
    robot_action_play_add = (robot_action_play_add+1)%840;//2*3*4*5=120
    return ret;
}

uint16_t robot_action_play_idx_get2()
{
    uint16_t ret = robot_action_play_add2;
    robot_action_play_add2 = (robot_action_play_add2+1)%840;//2*3*4*5=120
    return ret;
}


/****
TH01的Action
****/

//char servo_str[128];
//int servo_mode[12];//0:松开 1:中位 2:左边 3:右边 4:顺序 5：逆序 6789a:临时松开，可恢复
//int servo_data[12];
//int servo_pwm[12];
int ts_data[20];

//extern void robot_action_th01_mode_write(int mode);
//extern void robot_action_th01_data_send();

void robot_action_th01_init()
{
    /*
    ts15_data[0] = 10;
    ts15_data[1] = 150;
    ts15_data[2] = 150;
    ts15_data[3] = 150;
    //robot_pwm_init();
    robot_action_th01_mode_write(0);
    //robot_action_th01_data_send();
    //DBG_PRINT("robot_action_th01_init ok",0);
    //robot_pwm_setall(servo_pwm);
    //robot_pwm_face_send(servo_pwm+4);
    */
}

/*
void robot_action_th01_mode_write(int mode)
{
    int i;
    DBG_PRINT("robot_main_mode_write",mode);
    for(i=0;i<12;i++)
    {
        switch(mode)
        {
        case 0://松开
            servo_mode[i] = 0;
            servo_data[i] = 0;
            break;
        case 1://中位
            servo_mode[i] = 1;
            servo_data[i] = 0;
            break;
        case 2://解锁
            servo_mode[i] = ((servo_mode[i]>=1)&&(servo_mode[i]<=5))?(servo_mode[i]+5):(servo_mode[i]);
            break;
        case 3://上锁
            servo_mode[i] = ((servo_mode[i]>=6)&&(servo_mode[i]<=10))?(servo_mode[i]-5):(servo_mode[i]);
            break;
        }
    }
}

void robot_action_th01_data_write(int type,u8 data[],int sz)
{
    int i,j;
    DBG_PRINT("action_data_type = ",type);
    HEXS_PRINT("action_data",data,sz);
    switch(type)
    {
        case 1:
        case 2:
            if(sz<12)
            {
                DBG_PRINT("BAD SIZE RETURN",sz);
                return ;
            }

            break;
        case 3:
        case 4:
            if(sz<24)
            {
                DBG_PRINT("BAD SIZE RETURN",sz);
                return ;
            }
            break;
        case 5:
            if(sz<19)
            {
                DBG_PRINT("BAD SIZE RETURN",sz);
                return ;
            }
            break;
        default:
            DBG_PRINT("BAD TYPE RETURN",type);
            return ;
    }
    for(i=0;i<12;i++)
    {
        switch(type)
        {
        case 1://左边
            servo_mode[i] = 2;
            servo_data[i] = data[i];
            break;
        case 2://右边
            servo_mode[i] = 3;
            servo_data[i] = data[i];
        case 3://顺序
        case 4://逆序
            servo_mode[i] = type+1;
            servo_data[i] = data[i*2]*100+data[i*2+1];
        break;
        case 5://uart & flash mode
            j = (i<9)?(0):(2);
            servo_mode[i] = 4;
            servo_data[i] = data[i+2+j]*2;
            break;
        }
    }
    if(sz>24)
    {
        for(i=0;i<4;i++)
        {
            ts15_data[i] = data[24+i*2]*100+data[24+i*2+1];
        }
    }
    if((type==5)||(type==6))
    {
        ts15_data[0] = data[0];
        ts15_data[1] = data[16];
        ts15_data[2] = data[17];
        ts15_data[3] = data[18];
    }

}

void robot_action_th01_data_write_one(int idx,int type,int data)
{
    DBG2_PRINT("robot_main_onedata_write",idx,type);

    if(idx>=12)
    {
        ts15_data[0] = type;
        ts15_data[idx-12+1] = data;
    }
    else
    {
        servo_mode[idx] = type+1;
        servo_data[idx] = data;
    }
}

int robot_action_th01_get_servo_pwm(int mode,int data)
{
    int pwm;
    switch(mode)
    {
        case 1:pwm = 1500;break;
        case 2:pwm = 1500-data*5;break;
        case 3:pwm = 1500+data*5;break;
        case 4:pwm = 500+data*5;break;
        case 5:pwm = 2500-data*5;break;
        default:pwm = 0;break;
    }
    return pwm;
}

void robot_action_th01_data_show()
{
    int i;
    for(i=0;i<12;i++)
    {
        sprintf(servo_str,"Servo %d >> %d %d",i+1,servo_mode[i],servo_data[i]);
        DBG_PRINT(servo_str,servo_pwm[i]);
    }
    for(i=0;i<4;i++)
    {
        DBG2_PRINT("tst15_data >>",i,ts15_data[i]);
    }
}

int eye_once = 0;
*/
void robot_action_th01_data_send()
{
    int i;
    //#if 1
    //servo_mode[1] = 0;
    //servo_mode[3] = 0;
   // servo_mode[7] = 0;
    //servo_mode[11] = 0;
    //#endif
    //for(i=0;i<12;i++)
    //{
    //    servo_pwm[i] = robot_action_th01_get_servo_pwm(servo_mode[i],servo_data[i]);
    //}
    //robot_pwm_ts15_send(ts15_data);
    //ROBOT_DELAY_2MS(5);
    //robot_pwm_face_send(&servo_pwm[4]);
    //ROBOT_DELAY_2MS(100);
    //robot_pwm_setall(servo_pwm);

}
/*
void robot_action_th01_eye_data_send()
{
    int i;
    #if 1
    servo_mode[7] = 0;
    servo_mode[11] = 0;
    #endif
    for(i=4;i<12;i++)
    {
        servo_pwm[i] = robot_action_th01_get_servo_pwm(servo_mode[i],servo_data[i]);
    }
    robot_pwm_face_send(&servo_pwm[4]);
    ROBOT_DELAY_2MS(5);
}
*/
//int auto_action_step = 0;

void TS15_IDSMoveSend2(int idst,int ided,uint8_t data[],uint8_t time[])
{
    //TS15_IDSTimeSend2(time);
    //mdelay(2);
    TS15_IDSPosiSend_asyn3(data,time,idst,ided);
    mdelay((ided-idst+1)*4+2);
    TS15_asynMove_Set();
    mdelay(2);
    //mdelay(2);
}

void TS15_IDSMoveSend3(int idst,int ided,uint8_t data[],uint8_t time)
{
    //TS15_IDSTimeSend2(time);
    //mdelay(2);
    TS15_IDSPosiSend_asyn4(data,time,idst,ided);
    mdelay((ided-idst+1)*4+2);
    TS15_asynMove_Set();
    mdelay(2);
    //mdelay(2);
}

void TS15_ID2MoveSend2(int id,int data1,int data2,int time1,int time2)
{
    uint8_t data[20],time[20];
    data[0] = data1;
    data[1] = data2;
    time[0] = time1;
    time[1] = time2;
    TS15_IDSMoveSend2(id,id+1,data,time);
}

int robot_action_th01_data_make()
{
    uint16_t i,j;
    int tm1,tm2;
    int ret = 1;
    R11_ActionBreak_Flag = 0;
    int ids[10];
    int moves[10];
    int times[10];
    for(i=0;i<aid;i++)
    {
        DBG2_PRINT("TH01 ActionTag : ",adata[i][0],adata[i][1]);
        if(adata[i][1]==0)//播放音频表情
        {
            int show_id = robot_action_get_show_id(adata[i]);
            if(show_id>0)
            {
                if(show_id == ROBOT_ACTION_SHOW_AUDIO_STOP)
                    robot_audio_play_break();
                if((show_id>=ROBOT_ACTION_SHOW_AUDIO_ST)&&
                    (show_id<ROBOT_ACTION_SHOW_AUDIO_ST+ROBOT_ACTION_SHOW_AUDIO_SZ))
                    robot_audio_voice_play(show_id - ROBOT_ACTION_SHOW_AUDIO_ST + 1);
                if((show_id>=ROBOT_ACTION_SHOW_MUSIC_ST)&&
                    (show_id<ROBOT_ACTION_SHOW_MUSIC_ST+ROBOT_ACTION_SHOW_MUSIC_SZ))
                    robot_audio_music_play(show_id - ROBOT_ACTION_SHOW_MUSIC_ST);
                if((show_id>=ROBOT_ACTION_SHOW_EMOJI_ST)&&
                    (show_id<ROBOT_ACTION_SHOW_EMOJI_ST+ROBOT_ACTION_SHOW_EMOJI_SZ))
                    robot_lcd_emoji_set(show_id - ROBOT_ACTION_SHOW_EMOJI_ST);
                ROBOT_DELAY_MS(20);
                continue;
            }
        }
        if(adata[i][0]==0)
        {
             continue;
        }
        #if 0
        TS15_IDSTimeSend(adata[i][0]);
        ROBOT_DELAY_2MS(2);
        TS15_IDSPosiSend_asyn5(&adata[i][2]);
        ROBOT_DELAY_2MS(2);
        //TS15_COMM_Test_Once();
        TS15_asynMove_Set();
        #else
        //TS15_IDSTimeSend(adata[i][0]);
        //ROBOT_DELAY_MS(5);
        for(j=0;j<TS15_NUM;j++)
        {
            ids[j] = j+1;
            moves[j] = adata[i][2+j];
            times[j] = adata[i][0];
        }
        TS15_IDSPosiSend_syn(ids,moves,times,TS15_NUM);
        ROBOT_DELAY_MS(5);
        #endif
        ROBOT_DELAY_MS(5);
        if(SLEEP_TIME_ADD)
            ROBOT_DELAY_MS(SLEEP_TIME_ADD);
        DBG_PRINT("sleep >>",adata[i][1]);
        for(j=1;j<adata[i][1];j++)
        {
            ROBOT_DELAY_MS(15);
        }
    }
    aid = 0;//动作做完了，清空aid，避免新的动作会叠加以前的动作
    DBG_PRINT("data_make ret = ",ret);
    //if(auto_action_step)
    //    auto_action_step++;
    return ret;
}


#include "asm/gpio.h"

void sound_unmute_set(int unmute)
{
    int gpio_unmute = 1-unmute;
    gpio_direction_output(IO_PORTH_02,gpio_unmute);//设置功放,set为1
    //gpio_direction_output(IO_PORTD_03,1);
}

enum
{
    TSID_MOUTH = 1,

};



void robot_action_ts15_set_one(int idx,int move)
{
    int move2 = ts15_servo_base[idx]+move*ts15_servo_dir[idx];
    TS15_IDMoveSend(idx,move2,80);
}

void robot_action_ts15_set_one2(int idx,int move,int time)
{
    int move2 = ts15_servo_base[idx]+move*ts15_servo_dir[idx];
    TS15_IDMoveSend(idx,move2,time);
}

void robot_action_ts01_set_one(int idx,int move)
{
    DBG2_PRINT("robot_action_ts01_set_one",idx,move);
    if(idx==0)
        TS01_IDSMoveSend2(1,12,move,20);
    else
        TS01_IDMoveSend(idx,move,20);
}

int *rl;
int rl_normal[]={2,1,1,2,2,1,1,2,2,1,2,2,1,2,2,1,2,2,1,2};
int rl_happy[]= {6,2,6,3,6,2,5,1,4,1,4,2,5,3,6,2,6,3,6,2};
int rl_cry[]= {-4,2,-4,2,-4,2,-3,2,-3,1,-3,1,-3,1,-4,1,-4,2,-3,1};
int rlsz = 10;
int rlidx = 0;
int face_pos[]={55,70,150,130};
int face_add[]={0,0,0,0};
int face_asz = 0;
int eye1 = 85,eye2 = 85;

//int eye_pos[]={TSSTDData[3],TSSTDData[4],TSSTDData[7],TSSTDData[8],TSSTDData[2],TSSTDData[6]};
int eye_pos[]={98,98,100,104,101,97};
void robot_action_face_eye_do(int code)
{
    int a=0,b=0,c=0,d=0;
    int c1,c2;
    DBG_PRINT("eyedo ",code);
    if(code<756)
    {
        // 3 7 6 6
        //a = (code/252)%3-1;
        b = (code/36)%7;
        //a = a*2;
        b = (b==3)?(0):((b<3)?(b-4):(b-2));//b*2;
        c1 = (code/6)%6-2;
        c2 = (code/1)%6;

        c = c1;
        d = c1;
        switch(c2)
        {
        case 2:
            c -= 4;
            break;
        case 3:
            c += 4;
            break;
        case 4:
            d -= 3;
            break;
        case 5:
            d += 3;
            break;
        }
    }
    DBG_PRINT("b = ",b);
    DBG2_PRINT("cd = ",c,d);
    ts15ctl_add(3,b);
    ts15ctl_add(4,c);
    ts15ctl_add(5,d);
    //DBG2_PRINT("ab = ",a,b);

    //TS01_ID2MoveSend(3,7,eye_pos[0]+a,eye_pos[2]+a,12);
    //TS01_ID2MoveSend(4,8,eye_pos[1]+b,eye_pos[3]+b,12);
    //TS01_ID2MoveSend(2,6,eye_pos[4]-c,eye_pos[5]+d,12);

}

int face_emoji = 0;
int neck_data_set = 0;
int neck_data = 0;
int last_neck_data = 0;
void face_emoji_set(int emoji)
{
    face_emoji = emoji;
    DBG_PRINT("face_emoji = ",face_emoji);
}

void robot_action_neck_debug(int dir,int move,int mtime,int stime)
{
    static int neck_posi = 0;
    int last_posi = neck_posi;
    if(dir==0)
        neck_posi = 0;
    else if(dir==1)
        neck_posi += move;
    else
        neck_posi -= move;
    DBG2_PRINT("neck_set = ",move,mtime);
    DBG2_PRINT("neck fromto",last_posi,neck_posi);
    TS15_IDMoveSend(1,TSSTDData[13+2]+neck_posi,mtime);
    mdelay(stime);
}

void robot_action_neck_debug_make(uint8_t data[],int sz)
{
    int i;
    for(i=0;i<sz;i+=4)
    {
        int dir = data[i+0];
        int move = data[i+1];
        int mt = data[i+2]*10;
        int st = data[i+3]*10;
        robot_action_neck_debug(dir,move,mt,st);
    }
}

void robot_action_neck_go(int ec)
{
    static int neck_posi = -24;
    static int neck_dir = 0;
    static int neck_next = 0;
    static int neck_last = 0;
    static int neck_2flag = 0;

    if(neck_posi>52)
    {
        neck_dir = 1;
        neck_next = 0;

    }
    if(neck_posi<-52)
    {
        neck_dir = 0;
        neck_next = 0;
    }

    if(neck_next)
    {
        int abs = neck_posi-neck_last;
        if(abs<0)
            abs = -abs;
        if(abs>=neck_next)
        {
            neck_dir = 1 - neck_dir;
            neck_next = 0;
        }
    }
    if(neck_next==0)
    {
        neck_last = neck_posi;
        neck_next = 32+robot_random(32);
        ec = 60;
        DBG2_PRINT("next = ",neck_last,neck_next);
    }
    //DBG2_PRINT("neck code = ",ec,(ec+4)/5);
    ec = (ec+4)/5;
    switch(ec)
    {
        case 0:
            neck_posi = 0;
            neck_posi = 0;
            break;
        case 1:
        case 2:
        case 3:
            if(neck_dir==0)
                neck_posi+=3;
            else
                neck_posi-=3;
            neck_2flag = 0;
            break;
        case 12:
            if(neck_dir==0)
                neck_posi+=4;
            else
                neck_posi-=4;
            neck_2flag = 0;
            break;
        default:
            if(neck_dir==0)
                neck_posi+=2;
            else
                neck_posi-=2;
            neck_2flag++;
            if(neck_2flag==3)
            {
                if(neck_dir==0)
                    neck_posi+=1;
                else
                    neck_posi-=1;
                neck_2flag = 1;
            }
            break;
    }

    //DBG2_PRINT("neck_posi = ",neck_posi,neck_dir);
    //extern void robot_action_ts_set_one(int idx,int move);
    //robot_action_ts_set_one(13,neck_posi/3);
    neck_data = (neck_posi+52+2)/4-52/4;
    ts15ctl_add(1,neck_data);
    //DBG2_PRINT("neck >>",ec,neck_data);
    //neck_data_set = 1;
    //DBG_PRINT("neck_data = ",neck_data);

    /*
    if(neck_data!=last_neck_data)
    {
        last_neck_data = neck_data;

    }
    */
    //mdelay(10);

}

int robot_action_neck_flag = 1;

extern int speak_eye_flag;
void robot_action_face_auto_once(int dir,int emoji_flag)
{
    int eye_add;
    int lastdir = 0;
    int ec;
    int ec2;
    uint8_t face_move[6]={0};
    DBG2_PRINT("face",dir,emoji_flag);
    if(lastdir!=dir)
    {
        rlidx = 0;
        lastdir = dir;
    }

    switch(face_emoji)
    {
        case 0:rl = &rl_normal;break;
        case 1:rl = &rl_happy;break;
        case 2:rl = &rl_cry;break;
    }


    switch(dir)
    {
    case 0:
        face_add[0] = 0;
        face_add[1] = 0;
        face_add[2] = 0;
        face_add[3] = 0;
        face_asz = 0;
        break;
    case 1:
        if(face_asz<7)
        {
            face_add[0] += rl[rlidx+1];
            face_add[1] += rl[rlidx];
            face_add[2] -= rl[rlidx+1];
            face_add[3] -= rl[rlidx];
            face_asz++;
        }
        break;
    case 2:
        if(face_asz>0)
        {
            face_add[0] -= rl[rlidx+1];
            face_add[1] -= rl[rlidx];
            face_add[2] += rl[rlidx+1];
            face_add[3] += rl[rlidx];
            face_asz--;
        }
        break;
    }
    rlidx+=2;
    if(rlidx>=rlsz)
        rlidx = 0;
    //DBG2_PRINT("face add ",face_add[0],face_add[1]);
    face_move[2] = face_pos[0] + face_add[0];
    face_move[3] = face_pos[1] + face_add[1];
    face_move[4] = face_pos[2] + face_add[2];
    face_move[5] = face_pos[3] + face_add[3];
    if(speak_face_flag)
    {
        ts15ctl_add(7,face_data_mid-face_add[0]*2);
        ts15ctl_add(6,face_data_mid-face_add[0]*2);
    }
    if(speak_eye_flag)
    {
        ec = RANDOM(12096);
        if(dir==0)
            ec = 756;
        if(ec<1512)
        {
            robot_action_face_eye_do(ec);
        }
    }
    if(robot_action_neck_flag)
    {
        ec2 = RANDOM(47);
        robot_action_neck_go(ec2+1);
    }
}

void robot_action_face_eye_rand(int mode)
{
    int ec;
    switch(mode)
    {
    case 0:
        ec = RANDOM(1512);
        break;
    case 1:
        ec = RANDOM(756);
        break;
    case 2:
        ec = 756;
        break;
    }
    DBG_PRINT("ec = ",ec);
    ts15ctl_clear();
    robot_action_face_eye_do(ec);
    ts15ctl_make();
}

void robot_action_ts_set_one(int idx,int move)
{
    //idx<=12)
    //    robot_action_ts01_set_one(idx,move);
    //else
    //DBG2_PRINT("ts set",idx,move);
    robot_action_ts15_set_one(idx,move);
}

void robot_action_ts_set_one2(int idx,int move,int time)
{
    //idx<=12)
    //    robot_action_ts01_set_one(idx,move);
    //else
    //DBG2_PRINT("ts set",idx,move);
    robot_action_ts15_set_one2(idx,move,time);
}


void robot_action_ts_eye_ocne(int type,int move)
{
    uint8_t eye_move[10]={0,0,140,103,100,103,65,100,110,101};
                         //147 108 110 105 58 95 100 106
    int eyex=0,eyey=0;
    int eyey2=0,eyey3=0;
    switch(type)
    {
    case 1:
        eyex = move-25;
        eyey = 0;
        break;
    case 2:
        eyex = 0;
        eyey = move-20;
        break;
    case 3:
        if(move<=50)
        {
            eyex = move;
            eyey = (eyex<=25)?((eyex*4+2)/5):(((50-eyex)*4+2)/5);
            eyey = -eyey;
        }
        else
        {
            eyex = 100-move;
            eyey = ( eyex<=25)?((eyex*4+2)/5):(((50-eyex)*4+2)/5);
        }
        eyex = eyex-25;
        //eyey = eyey-20;
        break;
    case 4:
        if(move<=20)
        {
            eyex = move;
            eyey = (eyex<=10)?((eyex*4+1)/2):(((20-eyex)*4+1)/2);
            eyey = -eyey;
        }
        else
        {
            eyex = 40-move;
            eyey = (eyex<=10)?((eyex*4+1)/2):(((20-eyex)*4+1)/2);
        }
        eyex = eyex-10;
        break;
    case 5:
        if(move<=50)
        {
            eyex = move;
            eyey = (eyex<=25)?((eyex*4+2)/5):(((50-eyex)*4+2)/5);

        }
        else
        {
            eyex = 100-move;
            eyey = ( eyex<=25)?((eyex*4+2)/5):(((50-eyex)*4+2)/5);
            eyey = -eyey;
        }
        eyex = eyex-25;
        //eyey = eyey-20;
        break;
    case 6:
        if(move<=20)
        {
            eyex = move;
            eyey = (eyex<=10)?((eyex*4+1)/2):(((20-eyex)*4+1)/2);
        }
        else
        {
            eyex = 40-move;
            eyey = (eyex<=10)?((eyex*4+1)/2):(((20-eyex)*4+1)/2);
            eyey = -eyey;
        }
        eyex = eyex-10;
        break;
    }
    if(eyey>=12)
    {
        eyey2 = 5;
        eyey3 = 6;
    }
    else if(eyey>=7)
    {
        eyey2 = eyey-7;
        eyey3 = eyey-7;
    }
    else if(eyey<=-12)
    {
        eyey2 = -3;
        eyey3 = -2;
    }
    else if(eyey<-7)
    {
        eyey2 = -(-eyey-7)/2;
        eyey3 = -(-eyey-7)/2;
    }

    DBG2_PRINT("eye xy = ",eyex,eyey);
    DBG2_PRINT("eye y23 = ",eyey2,eyey3);
    eye_move[2]+=eyey2;
    eye_move[3]+=eyey3;
    eye_move[4]+=eyey;
    eye_move[5]+=eyex;

    eye_move[6]-=eyey2;
    eye_move[7]-=eyey3;
    eye_move[8]-=eyey;
    eye_move[9]+=eyex;

    //HEXS_PRINT("face_move = ",face_move,10);
    TS01_IDSMoveSend2(1,8,eye_move,4);
}

uint8_t face_move_std[6]={0,0,113,147,94,56};
int face_con_a=0,face_con_b=0,face_con_c=1;

void robot_action_ts_emoji_conset(int a,int b,int c)
{
    if(a==1)
        face_con_a = 1;
    else if(a==2)
        face_con_a = -1;
    else
        face_con_a = 0;
    face_con_b = b;
    if(c==0) c = 1;
    face_con_c = c;
    DBG_PRINT("con_set >> ",face_con_a);
    DBG2_PRINT("con_set2 >>",face_con_b,face_con_c);
}

int face_time_max=0,face_time_min=0,face_time_step=0,face_time_sleep=0;
int face_time_use = 0;

void robot_action_ts_emoji_timeset(int maxt,int mint,int stept,int sleept)
{
    face_time_max = maxt;
    face_time_min = mint;
    face_time_step = stept;
    face_time_sleep = sleept;
    DBG2_PRINT("Time max&min = ",face_time_max,face_time_min);
    DBG2_PRINT("Time step&sleep = ",face_time_step,face_time_sleep);
}

void robot_action_ts_emoji_set(uint8_t face[],int doflag)
{
    int i;
    if(doflag)
    {
        DBG2_PRINT("emoji_do = ",face[0],face[1]);
        DBG2_PRINT("emoji_do = ",face[2],face[3]);
    }
    else
    {
        DBG2_PRINT("emoji_set = ",face[0],face[1]);
        DBG2_PRINT("emoji_set = ",face[2],face[3]);
    }
    for(i=0;i<4;i++)
        face_move_std[i+2] = face[i];
    if(doflag)
        TS01_IDSMoveSend2(9,12,face_move_std,1);
}

void robot_action_ts_emoji_once(int type,int move)
{
    int i;
    uint8_t face_move_now[6];
    int move2 = face_con_a*(move*face_con_b+face_con_c/2)/face_con_c;
    DBG_PRINT("emoji set",type);
    DBG2_PRINT("emoji_move",move,move2);
    DBG2_PRINT("time m&s = ",face_time_use,face_time_sleep);
    for(i=2;i<6;i++)
        face_move_now[i] = face_move_std[i];
    switch(type)
    {
    case 1:
        face_move_now[2]-=move;
        face_move_now[3]-=move2;
        face_move_now[4]+=move;
        face_move_now[5]+=move2;
        break;
    case 2:
        face_move_now[2]-=move2;
        face_move_now[3]-=move;
        face_move_now[4]+=move2;
        face_move_now[5]+=move;
        break;
    case 3:
        face_move_now[2]+=move;
        face_move_now[3]+=move2;
        face_move_now[4]-=move;
        face_move_now[5]-=move2;
        break;
    case 4:
        face_move_now[2]+=move2;
        face_move_now[3]+=move;
        face_move_now[4]-=move2;
        face_move_now[5]-=move;
        break;

    }
    TS01_IDSMoveSend2(9,12,face_move_now,face_time_use);
    ROBOT_DELAY_MS(face_time_sleep);
}

#if 1
uint8_t eye2_move_std[6]={0,0,150,103,65,100};

void robot_action_ts_eye2_set(uint8_t eye2[],int doflag)
{
    eye2_move_std[2] = eye2[0];
    eye2_move_std[4] = eye2[1];
    if(doflag)
    {
        TS01_ID2MoveSend(1,2,eye2_move_std[2],eye2_move_std[3],4);
        TS01_ID2MoveSend(5,6,eye2_move_std[4],eye2_move_std[5],4);
    }

}

void robot_action_ts_eye2_once(int type,int move)
{
    int i;
    uint8_t eye2_move_now[6];
    int move2 = (move<=4)?(0):((move>=16)?(12):(move-4));
    for(i=2;i<6;i++)
        eye2_move_now[i] = eye2_move_std[i];
    switch(type)
    {
    case 1:
        eye2_move_now[2]+=move;
        eye2_move_now[3]+=(move2+2)/4;
        break;
    case 2:
        eye2_move_now[4]-=move;
        eye2_move_now[5]+=(move+2)/3;
        break;
    case 3:
        eye2_move_now[2]+=move;
        eye2_move_now[3]+=(move2+2)/4;
        eye2_move_now[4]-=move;
        eye2_move_now[5]+=(move+2)/3;
        break;
    }
    TS01_ID2MoveSend(1,2,eye2_move_now[2],eye2_move_now[3],4);
    TS01_ID2MoveSend(5,6,eye2_move_now[4],eye2_move_now[5],4);
}
#endif

//th01_action
void robot_action_th01_ts_once(int idx,int st,int ed,int step,int steptime)
{
    int j,k;
    int flag = 1;
    int dir = 1;
    u32 ts,tt;
    if(st>ed)
    {
        //int temp = st;
        //st = ed;
        //ed = temp;
        step = -step;
        flag = -1;
        dir = 2;

    }
    robot_main_wait_set(10);
    robot_main_wait_do();//时间对其
    ts15ctl_settimes(2);
    for(j=st;flag*j<=flag*ed;j+=step)
    {

        k = steptime;
        face_time_use = face_time_max;
        while(k--)
        {
            switch(idx)
            {
            case 16:
                //ts = robot_main_get_ms();
                DBG_PRINT("ts set 16 >> j = ",j);
                robot_main_wait_set(20);
                ts15ctl_clear();
                if(speak_face_flag || speak_eye_flag)
                    robot_action_face_auto_once(dir,1);
                {
                    extern int speak_mouth_flag;
                    if(speak_tone_flag)
                    {
                        if((j>=4)&&(j<=7))
                            ts15ctl_add(8,(j-4)*1);//robot_action_ts_set_one2(8,(j-4)*1,80);
                        else if(j<4)
                            ts15ctl_add(8,0);//robot_action_ts_set_one2(8,0,80);
                        else
                            ts15ctl_add(8,3);//robot_action_ts_set_one2(8,3,80);
                        //mdelay(2);
                    }
                    if(speak_mouth_flag)
                    {
                        ts15ctl_add(2,j);
                    }
                }
                ts15ctl_make();
                robot_main_wait_do();

                //tt = robot_main_get_ms();
                //DBGU32_PRINT("make time = ",tt-ts);
                break;
            case 17://眼睛左右
            case 18://眼睛上下
            case 19://眼睛转圈
                //11/12/13 01 00 14/28/32/64 01 01 00
                robot_action_ts_eye_ocne(idx-17+1,j);
                ROBOT_DELAY_MS(10);
                break;
            case 20:
            case 21:
            case 22:
            case 23:
                robot_action_ts_emoji_once(idx-20+1,j);
                break;
            case 24:
            case 25:
            case 26:
                robot_action_ts_eye2_once(idx-22+1,j);
                ROBOT_DELAY_MS(10);
                break;
            default:
                robot_action_ts_set_one(idx,j);
                ROBOT_DELAY_MS(10);
            break;
            }

        }
        face_time_use -= face_time_step;
        if(face_time_use<face_time_min)
            face_time_use = face_time_min;
    }
}

/*
void robot_action_th01_ts_once2(int gsz,int gdata[][])
{
    int j,k;
    if(st<ed)
    {
        for(j=st;j<=ed;j+=step)
        {
            k = steptime;
            while(k--)
            {
                robot_action_ts_set_one(idx,j);
                ROBOT_DELAY_2MS(5);
            }
        }
    }
    else
    {
        for(j=st;j>=ed;j-=step)
        {
            k = steptime;
            while(k--)
            {
                robot_action_ts_set_one(idx,j);
                ROBOT_DELAY_2MS(5);
            }
        }
    }
}
*/


void robot_action_th01_ts_step_set(int idx,int time,int st,int ed,int step,int steptime,int sleeptime)
{
    int i;
    for(i=0;i<time;i++)
    {
        if(i%2==0)
        {
            robot_action_th01_ts_once(idx,st,ed,step,steptime);
        }
        else
        {
            robot_action_th01_ts_once(idx,ed,st,step,steptime);
        }
    }
    msleep(sleeptime);
}


void robot_action_th01_ts_steps_set(int idx,uint8_t data[],int sz)
{
    int i;
    //robot_action_th01_mode_write(0);
    for(i=0;i<sz;i+=6)
    {
        if(sz-i>=6)
        {
            if((data[i]==0)&&(data[i+1]==0))
            {
                switch(data[i+2])
                {
                    case 0:robot_audio_voice_play(data[i+3]);break;
                    //case 1:robot_action_th01_eye_set(data[i+3],data[i+4]*100+data[i+5]);break;
                }
            }
            else
            {
                robot_action_th01_ts_step_set(idx,data[i+0],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5]);
            }
        }
    }
}

int wink_data[4][3]=
{
    //左开
    {101,85,98},
    //左闭
    {104,80,88},
    //右开
    {97,85,100},
    //右闭
    {102,80,110},
};

int wink_mtime = 1;
int wink_stime = 60;



void wink_data_range_set(int lid_range,int brow_range,int ball_range)
{
    wink_data[1][0] = wink_data[0][0] - lid_range ;
    wink_data[1][1] = wink_data[0][1] - brow_range;
    wink_data[1][2] = wink_data[0][2] - ball_range;

    wink_data[3][0] = wink_data[2][0] + lid_range + lid_range/2;
    wink_data[3][1] = wink_data[2][1] - brow_range;
    wink_data[3][2] = wink_data[2][2] + ball_range;

    DBG_PRINT("WINK_RANGE(lid) = ",lid_range);//眼帘
    DBG_PRINT("WINK_RANGE(brow) = ",brow_range);//眉毛
    DBG_PRINT("WINK_RANGE(ball) = ",ball_range);//眼球

}



void robot_action_wink_play(int type,int time1,int time2)
{
    DBG_PRINT("wink>>",type);
    DBG2_PRINT("wink>>",time1,time2);
    ts15ctl_clear();
    switch(type)
    {
    //左眼
    case 0x11:
        robot_action_ts_set_one2(4,4,time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 0x12:
        robot_action_ts_set_one2(4,0,time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 0x13:
        robot_action_ts_set_one2(4,4,time1);
        ROBOT_DELAY_MS(time2);
        robot_action_ts_set_one2(4,0,time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 0x14:
        robot_action_ts_set_one2(4,0,time1);
        ROBOT_DELAY_MS(time2);
        robot_action_ts_set_one2(4,6,time1);
        ROBOT_DELAY_MS(time2);
        break;
    //右眼
    case 0x21:
        robot_action_ts_set_one2(5,4,time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 0x22:
        robot_action_ts_set_one2(5,0,time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 0x23:
        robot_action_ts_set_one2(5,4,time1);
        ROBOT_DELAY_MS(time2);
        robot_action_ts_set_one2(5,0,time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 0x24:
        robot_action_ts_set_one2(5,0,time1);
        ROBOT_DELAY_MS(time2);
        robot_action_ts_set_one2(5,4,time1);
        ROBOT_DELAY_MS(time2);
        break;
    //双眼
    case 0x31:
        #if 0
        ts15ctl_settimes(time1);
        ts15ctl_clear();
        ts15ctl_add(4,4);
        ts15ctl_add(5,4);
        ts15ctl_make();
        #else
        robot_action_ts_set_one2(4,4,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(5,4,time1);
        ROBOT_DELAY_MS(5);
        #endif
        ROBOT_DELAY_MS(time2);
        break;
    case 0x32:
        #if 0
        ts15ctl_settimes(time1);
        ts15ctl_clear();
        ts15ctl_add(4,0);
        ts15ctl_add(5,0);
        ts15ctl_make();
        #else
        robot_action_ts_set_one2(4,0,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(5,0,time1);
        ROBOT_DELAY_MS(5);
        #endif
        ROBOT_DELAY_MS(time2);
        break;
    case 0x33:
        #if 0
        ts15ctl_settimes(time1);
        ts15ctl_clear();
        ts15ctl_add(4,4);
        ts15ctl_add(5,4);
        ts15ctl_make();
        #else
        robot_action_ts_set_one2(4,5,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(5,5,time1);
        ROBOT_DELAY_MS(5);
        #endif
        ROBOT_DELAY_MS(time2);
        #if 0
        ts15ctl_clear();
        ts15ctl_add(4,0);
        ts15ctl_add(5,0);
        ts15ctl_make();
        ROBOT_DELAY_MS(time2);
        #else
        robot_action_ts_set_one2(4,0,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(5,0,time1);
        ROBOT_DELAY_MS(5);
        #endif
        break;
    case 0x34:
        #if 0
        ROBOT_DELAY_MS(time2);
        ts15ctl_clear();
        ts15ctl_add(4,0);
        ts15ctl_add(5,0);
        ts15ctl_make();
        #else
        robot_action_ts_set_one2(4,0,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(5,0,time1);
        ROBOT_DELAY_MS(5);
        #endif
        #if 0
        ts15ctl_clear();
        ts15ctl_add(4,0);
        ts15ctl_add(5,0);
        ts15ctl_make();
        #else
        robot_action_ts_set_one2(4,4,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(5,4,time1);
        ROBOT_DELAY_MS(5);
        #endif
        ROBOT_DELAY_MS(time2);
        break;
    case 0x43:
        robot_action_ts_set_one2(4,16,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(6,face_data_mid-30,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(2,8,60);
        ROBOT_DELAY_MS(80);
        robot_action_ts_set_one2(8,28,240);
        ROBOT_DELAY_MS(time2+120);
        robot_action_ts_set_one2(8,0,280);
        ROBOT_DELAY_MS(200);
        robot_action_ts_set_one2(2,0,80);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(4,0,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(6,face_data_mid,time1);
        ROBOT_DELAY_MS(5);
        ROBOT_DELAY_MS(time2);
        break;
    case 0x53:
        robot_action_ts_set_one2(5,16,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(7,face_data_mid-30,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(2,8,60);
        ROBOT_DELAY_MS(80);
        robot_action_ts_set_one2(8,24,240);
        ROBOT_DELAY_MS(time2+80);
        robot_action_ts_set_one2(8,2,300);
        ROBOT_DELAY_MS(240);
        robot_action_ts_set_one2(2,0,80);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(5,0,time1);
        ROBOT_DELAY_MS(5);
        robot_action_ts_set_one2(7,face_data_mid,time1);
        ROBOT_DELAY_MS(5);
        ROBOT_DELAY_MS(time2);
        break;
    }
    ts15ctl_make();
}

int wink_data2[4][3]=
{
    //左开
    {101,85,98},
    //左闭
    {67,80,88},
    //右开
    {97,85,100},
    //右闭
    {148,80,110},

};

void robot_action_wink_play_debug(int type,int time1,int time2,int lid_range,int brow_range,int ball_range)
{
    wink_data2[1][0] = wink_data2[0][0] - lid_range ;
    wink_data2[1][1] = wink_data2[0][1] - brow_range;
    wink_data2[1][2] = wink_data2[0][2] - ball_range;

    wink_data2[3][0] = wink_data2[2][0] + lid_range + lid_range/2;
    wink_data2[3][1] = wink_data2[2][1] - brow_range;
    wink_data2[3][2] = wink_data2[2][2] + ball_range;
    DBG_PRINT("WINK TYPE = ",type);
    DBG2_PRINT("WINK_TIME = ",time1,time2);
    DBG2_PRINT("WINK_RANGE(lid,ball) = ",lid_range,ball_range);
    DBG_PRINT("WINK_RANGE(brow) = ",brow_range);
    switch(type)
    {
    //左眼
    case 11:
        TS01_IDMoveSend(2,wink_data2[0][0],time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 12:
        TS01_IDMoveSend(2,wink_data2[1][0],time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 13:
        TS01_ID2MoveSend(1,2,wink_data2[1][1],wink_data2[1][0],time1);
        ROBOT_DELAY_MS(time2);
        TS01_ID2MoveSend(1,2,wink_data2[0][1],wink_data2[0][0],time1);
        ROBOT_DELAY_MS(time2);
        break;
    //右眼
    case 21:
        TS01_IDMoveSend(6,wink_data2[2][0],time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 22:
        TS01_IDMoveSend(6,wink_data2[3][0],time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 23:
        TS01_ID2MoveSend(5,6,wink_data2[3][1],wink_data2[3][0],time1);
        ROBOT_DELAY_MS(time2);
        TS01_ID2MoveSend(5,6,wink_data2[2][1],wink_data2[2][0],time1);
        ROBOT_DELAY_MS(time2);
        break;
    //双眼
    case 31:
        TS01_ID2MoveSend(2,6,wink_data2[0][0],wink_data2[2][0],time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 32:
        TS01_ID2MoveSend(2,6,wink_data2[1][0],wink_data2[3][0],time1);
        ROBOT_DELAY_MS(time2);
        break;
    case 33:
        TS01_ID2MoveSend(2,6,wink_data2[1][0],wink_data2[3][0],time1);
        TS01_ID2MoveSend(3,7,wink_data2[1][2],wink_data2[3][2],time1);
        ROBOT_DELAY_MS(time);
        TS01_ID2MoveSend(2,6,wink_data2[0][0],wink_data2[2][0],time1);
        TS01_ID2MoveSend(3,7,wink_data2[0][2],wink_data2[2][2],time1);
        ROBOT_DELAY_MS(time);
        break;
    }
}


void robot_action_wink_play_debug_enter(uint8_t data[],int sz)
{
    int i;
    if(sz!=6)
    {
        DBG_PRINT("Data Length Err,length = ",sz);
        return ;
    }
    robot_action_wink_play_debug(data[0],data[1],data[2]*10,data[3],data[4],data[5]);
}

void robot_action_th01_ts_steps_set2(uint8_t data[],int sz)
{
    int i;
    //robot_action_th01_mode_write(0);

    for(i=0;i<sz;i+=7)
    {
        if(sz-i>=7)
        {
            if((data[i]==0)&&(data[i+1]==0))
            {
                switch(data[i+2])
                {
                    case 0:
                        robot_audio_voice_play(data[i+3]);
                        break;
                    case 1://wink
                        robot_action_wink_play(data[i+3],data[i+4],data[i+5]);
                        break;
                    case 2://delay
                        ROBOT_DELAY_MS(data[i+3]*200+data[i+4]*2);
                        break;
                    case 3:
                        robot_action_face_auto_once(0,1);
                        break;
                    case 4:
                        robot_audio_play_break();
                        robot_audio_wait_stop();
                        break;
                    case 5:
                    case 6:
                        robot_action_ts_emoji_set(&data[i+3],data[i+2]-5);
                        break;
                    case 7:
                        robot_action_ts_emoji_conset(data[i+3],data[i+4],data[i+5]);
                        break;
                    case 8:
                        robot_action_ts_emoji_timeset(data[i+3],data[i+4],data[i+5],data[i+6]);
                        break;
                    case 9:
                    case 10:
                        robot_action_ts_eye2_set(&data[i+3],data[i+2]-8);
                        break;
                    case 11://read
                        TS01_IDSMoveRead(1,12,NULL);
                        break;
                    //case 1:robot_action_th01_eye_set(data[i+3],data[i+4]*100+data[i+5]);break;
                }
            }
            else
            {
                robot_action_th01_ts_step_set(data[i+0],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6]);
            }
        }
    }
}

void robot_action_th01_speak(int size,int time1,int time2,int sleep)//幅度，跳格，停次
{
    u8 data[8];
    extern int mouth_debug_mode;
    if(mouth_debug_mode)
    {
        DBG2_PRINT("speak size&frame = ",size,(size/time1+1)*time2);
        DBG2_PRINT("speak time = ",time1,time2);
    }
    data[0] = 0x10;
    data[1] = 0x02;
    data[2] = 0x00;
    data[3] = size;
    data[4] = time1;
    data[5] = time2;
    data[6] = sleep;
    robot_action_th01_ts_steps_set2(data,7);
}

void TS15_IDUserRead(int id,int add,int length)
{
   	uint8_t para[6];
    uint8_t comm[12];
    uint8_t csz;
    uint16_t ret = 0;
    int old_uart_debug = 0;
    int ts15_id = GET_TS15_ID(id);
    DBG_PRINT("TS15_IDPosiRead",ts15_id);
    para[0] = length;

    csz = robot_bus_command_make(ts15_id,0x02,add,1,para,comm);
    BUS_FLUSH();
    robot_bus_read_flag_set(BUS_RF_TS15_USER,length,25);
    old_uart_debug = robot_uart_bus_debug_get();
    robot_uart_bus_debug_set(UART_DBG_ON);
    BUS_WRITE(comm,csz);
    BUS_SETIN();
    ROBOT_DELAY_MS(2);
    while(BUS_READING)//UART_INPUT_HANDLEÄÚ´øÓÐ³¬Ê±ÅÐ¶Ï
    {
        robot_bus_data_read_once();
        ROBOT_DELAY_MS(2);
    }
    BUS_SETOUT();
    robot_uart_bus_debug_set(old_uart_debug);
}

void TS15_IDUserWrite(int idx,int add,int length,uint8_t data[])
{
    uint8_t para[100];
    uint8_t comm[100];
    uint16_t csz;
    int i,j = 0;
    para[j++]=length;
    idx = GET_TS15_ID(idx);
    para[j++] = idx;
    for(i=0;i<length;i++)
       para[j++] = data[i];
    csz = robot_bus_command_make(0xfe,0x83,add,j,para,comm);
    BUS_WRITE(comm,csz);
    ROBOT_DELAY_MS(2);
}



/*
void robot_action_th01_ts_steps_set3(uint8_t data[],int sz)
{
    int i;
    //robot_action_th01_mode_write(0);
    //for(i=0;i<sz;i+=7)
    for(i=0;i<sz;)
    {
        if(sz-i>=7)
        {
            if((data[i]==0)&&(data[i+1]==0))
            {
                switch(data[i+2])
                {
                    case 0:robot_audio_voice_play(data[i+3]);break;
                    //case 1:robot_action_th01_eye_set(data[i+3],data[i+4]*100+data[i+5]);break;
                }
            }
            else
            {
                robot_action_th01_ts_step_set(data[i+0],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6]);
            }
        }
    }
}
*/
