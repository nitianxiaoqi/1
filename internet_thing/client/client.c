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
    if (-1 == connect(netfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        perror("connect failed");
        return -1;
    }
    memset(&buf, 0, sizeof(Netdata_t));
    printf("请输入账号：");
    scanf("%s", buf.ID);
    printf("请输入密码：");
    scanf("%s", buf.PS);
    send(netfd, &buf, sizeof(Netdata_t), 0);
    recv(netfd, &buf, sizeof(Netdata_t), 0);

    /*设备初始化  本质是将设备文件全部打开*/
    if (dev_init())
    {
        puts("设备初始化失败");
        close(netfd);
        return -1;
    }

    if (buf.flags)
    {
        pthread_create(&tid, NULL, usersetthread, NULL);
        // 等待服务器指令
        while (1)
        {
            puts("等待指令");
            printf("Netdata_t : %ld\n", sizeof(Netdata_t));
            printf("netfd = %d\n", netfd);
            printf("&buf = %p\n", &buf);
            if (0 == recv(netfd, &buf, sizeof(Netdata_t), 0))
            {
                puts("服务器连接错误");
                close(netfd);
                return -1;
            }
            printf("comd : %d\n", buf.msg.comd);
            // 处理服务器指令多线程处理
            switch (buf.msg.comd)
            {
            case 1:
                /*获取环境数据*/
                printf("11111\n");
                pthread_create(&tid, NULL, getenvthread, NULL);
                printf("获取环境数据指令到来\n");
                pthread_detach(tid);
                break;
            case 2:
                /*设置阈值*/
                pthread_create(&tid, NULL, limitsetthread, &(buf.msg));
                printf("阈值设置指令到来\n");
                pthread_detach(tid);
                break;
            case 3:
                /*控制设备*/
                pthread_create(&tid, NULL, devctrlthread, &(buf.msg));
                printf("设备控制设置指令到来\n");
                pthread_detach(tid);
                break;
            default:
                break;
            }
            memset(&buf, 0, sizeof(Netdata_t));
        }
    }
    else
    {
        close(netfd);
        return 0;
    }

    return 0;
}