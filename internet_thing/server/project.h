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
#include <stdbool.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sqlite3.h>

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
    long msgtype;
    long recvtype;     // 等待消息类型
    char userinfo[20]; // 上位机用户信息字段
    char flags;        // 用于标识下位机是否存在   1存在，0不存在
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

typedef struct
{
    char id[5];
    char username[20];
    char password[7];
    int flags;
} loginInfo_t;

// 链表节点结构
typedef struct node
{
    char name[20];
    int fd;
    struct node *next;
} Node_t;

Netdata_t buf;
Node_t *L;
key_t key;
int msqid;
pthread_t tid;
loginInfo_t loginInfo;
// 网络初始化
int Net_init(const char *IP, const char *PORT);

// 消息队列初始化
int msgq_init(void);

// 链表初始化
Node_t *linklist_init(void);

// 头部插入
int hendinsert(Node_t *L, const char *username, int fd);

// 根据ID查找
int namefind(Node_t *L, const char *username);

// 根据文件描述符删除
int filedelete(Node_t *L, int fd);

// 遍历链表
void showlist(Node_t *L);

// 释放链表
int freelist(Node_t **L);

// 上位机检索下位机线程
void *handler_uprequest(void *argv);

// 下位机处理线程
void *handler_downrequest(void *argv);

#endif
