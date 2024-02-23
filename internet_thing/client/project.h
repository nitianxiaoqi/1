#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <sys/ipc.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/ioctl.h>
#include "si7006.h"
#include <fcntl.h>

#define MAX_CLIENTS 10
#define MSGPATH "/home/linux"

int netfd, clientfd;
struct sockaddr_in server_addr, clientadd;

typedef struct
{
    float tempvalue;
    unsigned char humevalue;
    unsigned short uillevalue;
    float volvalue;
    char ledstate;
    char fanstate;
    char humestate;
    char beepstate;

} Envdata_t;

typedef struct
{
    float tempup;
    float tempdown;
    unsigned char humeup;
    unsigned char humedown;
    unsigned short iullup;
    unsigned short iulldown;
} Limitset_t;

typedef struct
{
    char led;
    char fan;
    char hume;
    char beep;
} Devctrl_t;

typedef struct
{

    long long msgtype;
    long long recvtype; // 等待消息类型
    char userinfo[20];  // 上位机用户信息字段
    char flags;         // 用于标识下位机是否存在   1存在，0不存在
    char comd;
    Envdata_t env;    // 环境数据
    Limitset_t limit; // 阈值设置
    Devctrl_t dev;    // 设备控制

} Msg_t;

typedef struct
{
    char ID[20];
    char PS[7];
    char flags;
    Msg_t msg;

} Netdata_t;

pthread_t tid;

Netdata_t buf;

float settempup;
float settempdown;
char sethumeup;
char sethumedown;
short setiullup;
short setiulldown;

float conttemp;
char conthume;
short contiull;
float contvol;

// 网络初始化
int Net_init(const char *IP, const char *PORT);

// 设备初始化
int dev_init(void);

/*维护用户环境设置线程*/
void *usersetthread(void *argv);

/*获取环境数据线程*/
void *getenvthread(void *argv);

/*阈值设置线程*/
void *limitsetthread(void *argv);

/*设备控制线程*/
void *devctrlthread(void *argv);

#endif
