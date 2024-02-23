#include "project.h"

int main(int argc, const char *argv[])
{
    if (3 != argc)
    {
        puts("请输入 IP地址 和 端口号");
        return -1;
    }

    // 网络初始化
    netfd = Net_init(argv[1], argv[2]);
    if (-1 == netfd)
    {
        puts("网络初始化失败");
    }

    // 消息队列初始化
    if (msgq_init())
    {
        puts("消息队列初始化失败");
        close(netfd);
        return -1;
    }

    // 链表初始化
    L = linklist_init();
    if (NULL == L)
    {
        puts("链表初始化失败");
        close(netfd);
        msgctl(msqid, IPC_RMID, NULL);
        return -1;
    }
    // 创建上位机检索线程(登录时检索链表中是否有下位机存在)
    pthread_create(&tid, NULL, handler_uprequest, NULL);

    puts("系统启动  等待下位机连接");
    int lenth = sizeof(struct sockaddr_in);
    // 等待下位机连接(有连接则创建线程处理连接)
    while (1)
    {
        clientfd = accept(netfd, (struct sockaddr *)&clientadd, &lenth);
        if (-1 == clientfd)
        {
            puts("连接错误");
        }
        printf("clientfd = %d\n", clientfd);
        // 创建下位机的处理线程
        pthread_create(&tid, NULL, handler_downrequest, &clientfd);

        pthread_detach(tid);
    }

    return 0;
}
