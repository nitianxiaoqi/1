#include "project.h"

char getbuf[4] = {0};

// alarm信号处理函数
void sighandler(int argc)
{
    msgrcv(msqid, &msg, sizeof(Msg_t) - sizeof(long), mrecv, IPC_NOWAIT);
    msgrcv(msqid, &msg, sizeof(Msg_t) - sizeof(long), mrecv * 2, IPC_NOWAIT);
    printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
    printf("<!DOCTYPE html>");
    printf("<html>");
    printf("<body>");
    printf("<center>");
    printf("<h2>网络原因导致数据不可达，请稍后刷新重试</h2>");
    printf("<a href=\"../home/living_room.html\"><h2>返回</h2></a>");
    printf("</center>");
    printf("</body>");
    printf("</html>");
    exit(-1);
}

int cgiMain(int argc, const char *argv[])
{

    // 信号初始化
    signal(SIGALRM, sighandler);

    // 消息队列初始化
    key = ftok("/home/linux", 'm');
    if (-1 == key)
    {
        return -1;
    }

    msqid = msgget(key, IPC_CREAT | 0666);
    if (-1 == msqid)
    {
        return -1;
    }
    memcpy(cookvalue, cgiCookie, 128);
    memset(&msg, 0, sizeof(Msg_t));

    strcpy(msg.userinfo, cookvalue);

    mrecv = 0;
    char *s = msg.userinfo;
    while (*s != '=')
    {
        s++;
    }
    s++;
    strcpy(loginInfo.username, s);

    while (*s)
    {
        mrecv += *s;
        s++;
    }

    cgiFormString("LED", getbuf, 4);
    if (strcmp("ON", getbuf))
    {
        msg.dev.led = 0;
    }
    else
    {
        msg.dev.led = 1;
    }
    memset(getbuf, 0, 4);
    cgiFormString("FAN", getbuf, 4);
    if (strcmp("ON", getbuf))
    {
        msg.dev.fan = 0;
    }
    else
    {
        msg.dev.fan = 1;
    }
    memset(getbuf, 0, 4);
    cgiFormString("BEEP", getbuf, 4);
    if (strcmp("ON", getbuf))
    {
        msg.dev.beep = 0;
    }
    else
    {
        msg.dev.beep = 1;
    }
    memset(getbuf, 0, 4);

    msg.msgtype = mrecv;
    msg.comd = 3; // 电器控制
    msg.recvtype = mrecv * 2;
    if (-1 == msgsnd(msqid, &msg, sizeof(Msg_t) - sizeof(long), 0))
    {
        return -1;
    }

    alarm(5);
    // 读取消息 等待结果
    memset(&msg, 0, sizeof(Msg_t));
    if (-1 == msgrcv(msqid, &msg, sizeof(Msg_t) - sizeof(long), mrecv * 2, 0))
    {
        return -1;
    }
    alarm(5);
    printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
    printf("<!DOCTYPE html>");
    printf("<html>");
    printf("<body>");
    printf("<center>");
    printf("<h2>电器控制成功</h2>");
    printf("<a href=\"../home/living_room.html\"><h2>返回</h2></a>");
    printf("</center>");
    printf("</body>");
    printf("</html>");
    return 0;
}
