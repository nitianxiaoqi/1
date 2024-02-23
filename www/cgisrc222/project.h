#ifndef __PROJECT_H__
#define __PROJECT_H__

#include "cgic.h"
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>


typedef struct{
     float tempvalue;
     unsigned char humevalue; 
     unsigned short uillevalue;
     float volvalue;
     char ledstate;
     char fanstate;
     char humestate;
     char beepstate;     

} Envdata_t;

typedef struct{
    float tempup;
    float tempdown;
    unsigned char humeup;
    unsigned char humedown;
    unsigned short iullup;
    unsigned short iulldown;
} Limitset_t;

typedef struct{
     char led;
     char fan;
     char hume;
     char beep;
} Devctrl_t; 

typedef struct{

	long msgtype;
	long recvtype;//等待消息类型
	char userinfo[20];//上位机用户信息字段
	char flags;//用于标识下位机是否存在   1存在，0不存在
	char comd; //用于下位机甄别操作指令使用  1:获取环境信息  2:设置阈值  3:设备控制	
	Envdata_t env;//环境数据
	Limitset_t limit;//阈值设置
	Devctrl_t dev;//设备控制

} Msg_t;

key_t key;
int msqid;
Msg_t msg;
long mrecv;
char cookvalue[128] = {0};



#endif
