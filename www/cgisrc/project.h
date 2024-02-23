#ifndef __PROJECT_H__
#define __PROJECT_H__

#include "cgic.h"
#include <sqlite3.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

typedef struct
{
    float tempvalue;           // 温度
    unsigned char humevalue;   // 湿度
    unsigned short uillevalue; // 光强
    float volvalue;            // 电压
    char ledstate;             // 光照设备状态
    char fanstate;             // 温控设备状态
    char humestate;            // 加湿设备状态
    char beepstate;            // 警报设备状态
} Envdata_t;

typedef struct
{
    float tempup;            // 温度上限
    float tempdown;          // 温度下限
    unsigned char humeup;    // 湿度上限
    unsigned char humedown;  // 湿度下限
    unsigned short iullup;   // 光强上限
    unsigned short iulldown; // 光强下限
} Limitset_t;

typedef struct
{
    char led;  // 光照设备
    char fan;  // 温控设备
    char hume; // 加湿设备
    char beep; // 警报设备
} Devctrl_t;

typedef struct
{
    long msgtype;      // 消息类型
    long recvtype;     // 消息接收类型
    char userinfo[20]; // 上位机用户信息字段
    char flags;        // 用于标识下位机是否存在(1存在，0不存在)
    char comd;         // 用于下位机甄别操作指令使用  1:获取环境信息  2:设置阈值  3:设备控制
    Envdata_t env;
    Limitset_t limit;
    Devctrl_t dev;
} Msg_t;

typedef struct
{
    char ID[20];
    char PS[7];
    char flags;
    Msg_t msg;

} Netdata_t;

typedef struct
{
    char id[5];
    char username[20];
    char password[7];
    int flags;
} loginInfo_t;

key_t key;
int msqid;
Msg_t msg;
long mrecv;
char cookvalue[128] = {0};
loginInfo_t loginInfo;

#endif
