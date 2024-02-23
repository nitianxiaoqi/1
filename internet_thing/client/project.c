#include "project.h"

// 网络初始化
int Net_init(const char *IP, const char *PORT)
{
    int fd = 0;
    if (-1 == (fd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        perror("socket failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port = htons(atoi(PORT));

    return fd;
}

// 设备初始化
int dev_init(void)
{
    /*打开设备文件*/

    return 0;
}

/*维护用户环境设置线程*/
void *usersetthread(void *argv)
{
    int fd;
    int rh, tem;
    float rh_data, tem_data;
    Netdata_t getbuf;
    if ((fd = open("/dev/si7006", O_RDWR)) == -1)
        perror("open error");

    ioctl(fd, GET_RH, &rh);
    ioctl(fd, GET_TEM, &tem);

    rh_data = 125.0 * rh / 65536 - 6;
    tem_data = 175.72 * tem / 65536 - 46.85;

    printf("rh_data = %.2f,tem_data = %.2f\r", rh_data, tem_data);
    usleep(500);
    fflush(stdout);
    send(netfd, &getbuf, sizeof(Netdata_t), 0);

    FILE *fp = fopen("./envdata.txt", "r+");
    char readdata[6][20] = {0};
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        fgets(readdata[i], 20, fp);
        switch (i)
        {
        case 0:
            settempup = atof(readdata[i]);
            break;
        case 1:
            settempdown = atof(readdata[i]);
            break;
        case 2:
            sethumeup = atoi(readdata[i]);
            break;
        case 3:
            sethumedown = atoi(readdata[i]);
            break;
        case 4:
            setiullup = atoi(readdata[i]);
            break;
        case 5:
            setiulldown = atoi(readdata[i]);
            break;
        }
    }

    fclose(fp);
    puts("维护阈值环境线程运行");
    while (1)
    {
        /*获取环境数据 与 设置值进行比较  来控制设备是否开启*/
        if (tem_data > settempup)
        {
        }
        sleep(1);
    }
}

/*获取环境数据线程*/
void *getenvthread(void *argv)
{
    puts("获取环境数据指令到来");
    Netdata_t getbuf;

    /*
     * 如果不考虑系统效率问题可以直接读取设备数据
     *如果考虑系统效率问题 则可以用usersetthread
     线程的获取的数据进行赋值即可
     *
     * */
    int fd;
    int rh, tem;
    float rh_data, tem_data;
    if ((fd = open("/dev/si7006", O_RDWR)) == -1)
        perror("open error");

    ioctl(fd, GET_RH, &rh);
    ioctl(fd, GET_TEM, &tem);

    rh_data = 125.0 * rh / 65536 - 6;
    tem_data = 175.72 * tem / 65536 - 46.85;

    printf("rh_data = %.2f,tem_data = %.2f\r", rh_data, tem_data);
    usleep(500);
    getbuf.msg.env.tempvalue = tem_data;
    getbuf.msg.env.humevalue = rh_data;
    fflush(stdout);
    send(netfd, &getbuf, sizeof(Netdata_t), 0);

    close(fd);
    /*将数据 发送给服务器后 退出即可*/

    pthread_exit(NULL);
}

/*阈值设置线程*/
void *limitsetthread(void *argv)
{
    puts("设置阈值指令到来");
    Netdata_t getbuf;
    Msg_t setbuf = *((Msg_t *)argv);

    settempup = setbuf.limit.tempup;
    settempdown = setbuf.limit.tempdown;
    printf("setbuf.limit.tempup = %d\n", setbuf.limit.tempup);
    printf("setbuf.limit.tempdown = %d\n", setbuf.limit.tempdown);

    FILE *fp = fopen("./envdata.txt", "w+");
    char readdata[6][20] = {0};
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        fputs(readdata[i], fp);
        switch (i)
        {
        case 0:
            memcpy(readdata[i], &settempup, 20);
            break;
        case 1:
            memcpy(readdata[i], &settempdown, 20);
            break;
        case 2:
            sethumeup = atoi(readdata[i]);
            break;
        case 3:
            sethumedown = atoi(readdata[i]);
            break;
        case 4:
            setiullup = atoi(readdata[i]);
            break;
        case 5:
            setiulldown = atoi(readdata[i]);
            break;
        }
    }

    fclose(fp);

    /*将网络数据的阈值  赋值给参考变量
     *将数据再写入到envdata.txt中
     *将执行结果返回给服务器
     *线程结束
     *
     * */

    send(netfd, &getbuf, sizeof(Netdata_t), 0);
    pthread_exit(NULL);
}

/*设备控制线程*/
void *devctrlthread(void *argv)
{
    Netdata_t getbuf;
    Msg_t devbuf = *((Msg_t *)argv);
    puts("控制设备指令到来");
    /*根据网络数据中的 设备控制值对设备进行设置
     *
     * 设置完毕
     *
     * 返回网络数据
     *
     * 线程退出
     *
     * */
    send(netfd, &getbuf, sizeof(Netdata_t), 0);
    pthread_exit(NULL);
}
