#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <fcntl.h>
#include <aio.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sqlite3.h>

#define PORT 8888
#define NUMBER_EVENTS 1024

// 命令码
#define CMD_LOGIN 0x1      // 登录命令码
#define CMD_REGIS 0x2      // 注册命令码
#define CMD_ADD 0x3        // 增加命令码
#define CMD_DEL 0x4        // 删除命令码
#define CMD_MODIFY 0x5     // 修改命令码
#define CMD_SERACH_ALL 0x6 // 查询命令码

#define CMD_ADD_OPTION 0x7    // 增加子命令码
#define CMD_DEL_OPTION 0x8    // 删除子命令码
#define CMD_MODIFY_OPTION 0x9 // 修改子命令码
#define CMD_SERACH_OPTION 0xA // 查询子命令码

// 登陆注册结构体
typedef struct
{
    char username[20];
    char password[7];
    int flags;
} login_t;
// 用户信息结构体
typedef struct
{
    int id;            // 序号
    char name[20];     // 姓名
    char sex[5];       // 性别
    char age[5];       // 年龄
    char post[20];     // 岗位
    char salary[20];   // 工资
    char phone[12];    // 电话
    char homeaddr[64]; // 家庭住址
} userinfo_t;
typedef struct
{
    char command;         // 指令字段
    login_t login;        // 登录信息结构体
    userinfo_t userinfo;  // 用户信息结构体
    char privatedata[20]; // 传递私有数据
    char result[128];     // 执行结果
} netdata_t;

int login_maxid = 0;
int info_maxid = 0;

int get_login_max_id(sqlite3 **msgSys_db);
int get_login_maxid_func(void *data, int argc, char **argv, char **azColName);
int get_info_max_id(sqlite3 **msgSys_db);
int get_info_maxid_func(void *data, int argc, char **argv, char **azColName);
int login_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata);
int register_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata);
int add_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata);
int delete_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata);
int modify_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata);
int add_option_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata);
int del_option_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata);
int modify_option_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata);
int serach_option_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata);
int serach_all_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata);

int main(int argc, const char *argv[])
{
    int ret;
    int listen_fd;
    sqlite3 *msgSys_db = NULL;
    netdata_t recv_netdata;
    char *errmsg = NULL;
    const char *sql;
    int server_len;
    int epoll_fd;
    int maxevents;
    // 创建一个数据库实例（数据库的文件）并打开。
    ret = sqlite3_open("./msgSys.db", &msgSys_db);
    if (ret != SQLITE_OK)
    {
        printf("数据库打开异常\n");
        return -1;
    }
    // 创建数据表：
    sql = "create table if not exists loginTable(序号 int ,用户名 text int primary key, 密码 text);";
    ret = sqlite3_exec(msgSys_db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("执行sql失败:%s\n", errmsg);
        return -1;
    }
    sql = "create table if not exists infoTable(序号 int primary key,姓名 text, 性别 text, 年龄 text, 岗位 text, 工资 text, 电话 text, 家庭住址 text);";
    ret = sqlite3_exec(msgSys_db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("执行sql失败:%s\n", errmsg);
        return -1;
    }
    // 查询登录表最大序号
    ret = get_login_max_id(&msgSys_db);
    if (ret != 0)
    {
        printf("查询登录表最大序号失败\n");
    }
    // 查询信息表最大序号
    ret = get_info_max_id(&msgSys_db);
    if (ret != 0)
    {
        printf("查询信息表最大序号失败\n");
    }
    // 创建监听套接字
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == listen_fd)
    {
        perror("socket error");
        return -1;
    }
    // 绑定套接字和结构体
    struct sockaddr_in server_info =
        {
            .sin_family = AF_INET,
            .sin_addr = INADDR_ANY,
            .sin_port = htons(PORT),
        };
    server_len = sizeof(server_info);
    ret = bind(listen_fd, (const struct sockaddr *)&server_info, server_len);
    if (-1 == ret)
    {
        perror("bind error");
        return -1;
    }
    // 监听
    ret = listen(listen_fd, 5);
    if (-1 == ret)
    {
        perror("listen error");
        return -1;
    }
    // 创建文件描述符集合epoll_fd
    epoll_fd = epoll_create1(0);
    // 使用一个结构体保存我们关心的文件描述符
    struct epoll_event ev =
        {
            .events = EPOLLIN,
            .data.fd = listen_fd,
        };
    // 把关心的文件描述符放到集合中
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);
    if (-1 == ret)
    {
        perror("ret error");
        return -1;
    }
    // 创建一个与内核双向链表对应的空间
    struct epoll_event order_fdarrar[NUMBER_EVENTS] = {0};
    maxevents = sizeof(order_fdarrar) / sizeof(struct epoll_event);
    // 把文件描述符交给内核
    // char buf[512] = {0};
    printf("epoll并发服务器启动\n");
    ret = 0;
    while (true)
    {
        // 接收信号,返回值为文件描述符
        int nfds = epoll_wait(epoll_fd, order_fdarrar, maxevents, -1);
        if (-1 == nfds)
        {
            perror("epoll_wait error");
            continue;
        }
        // 遍历返回的描述符集合，找到有事件的描述符
        for (int i = 0; i < nfds; i++)
        {
            if (order_fdarrar[i].data.fd == listen_fd)
            { // 有新的连接请求
                int connect_fd = accept(order_fdarrar[i].data.fd, NULL, NULL);
                if (-1 == connect_fd)
                {
                    perror("accept error");
                    continue;
                }
                // 把新的文件描述符放到集合中
                ev.data.fd = connect_fd;
                ev.events = EPOLLIN;
                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &ev);
                if (-1 == ret)
                {
                    perror("添加失败\n");
                    continue;
                }
            }
            else
            { // 有新发来的消息
                // memset(buf, 0, sizeof(buf));
                ret = recv(order_fdarrar[i].data.fd, &recv_netdata, sizeof(recv_netdata), 0);
                if (-1 == ret)
                {
                    perror("recv error");
                    continue;
                }
                if (0 == ret)
                {
                    printf("对端关闭\n");
                    ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, order_fdarrar[i].data.fd, &order_fdarrar[i]);
                    if (-1 == ret)
                    {
                        perror("epoll_ctl error");
                        return -1;
                    }
                    close(order_fdarrar[i].data.fd);
                    continue;
                }
                // 处理客户端的请求
                switch (recv_netdata.command)
                {
                case CMD_LOGIN:
                    login_databases(&msgSys_db, &order_fdarrar[i].data.fd, recv_netdata);
                    break;
                case CMD_REGIS:
                    register_databases(&msgSys_db, &order_fdarrar[i].data.fd, recv_netdata);
                    break;
                case CMD_ADD:
                    add_databases(&msgSys_db, &order_fdarrar[i].data.fd, recv_netdata);
                    break;
                case CMD_DEL:
                    delete_databases(&msgSys_db, &order_fdarrar[i].data.fd, recv_netdata);
                    break;
                case CMD_MODIFY:
                    modify_databases(&msgSys_db, &order_fdarrar[i].data.fd, recv_netdata);
                    break;
                case CMD_ADD_OPTION:
                    add_option_databases(&msgSys_db, &order_fdarrar[i].data.fd, recv_netdata);
                    break;
                case CMD_DEL_OPTION:
                    del_option_databases(&msgSys_db, &order_fdarrar[i].data.fd, recv_netdata);
                    break;
                case CMD_MODIFY_OPTION:
                    modify_option_databases(&msgSys_db, &order_fdarrar[i].data.fd, recv_netdata);
                    break;
                case CMD_SERACH_OPTION:
                    serach_option_databases(&msgSys_db, &order_fdarrar[i].data.fd, recv_netdata);
                    break;
                case CMD_SERACH_ALL:
                    serach_all_databases(&msgSys_db, &order_fdarrar[i].data.fd, recv_netdata);
                    break;
                }
            }
        }
    }
    close(listen_fd);
    return 0;
}

// 查询登陆表最大序号
int get_login_max_id(sqlite3 **msgSys_db)
{
    char *sql_insert = NULL;
    char *errMsg = NULL;
    int ret = 0;
    sql_insert = "select MAX(序号) from loginTable;";
    ret = sqlite3_exec(*msgSys_db, sql_insert, get_login_maxid_func, NULL, &errMsg);
    if (SQLITE_OK != ret)
    {
        printf("查询序号失败:%s\n", errMsg);
        return -1;
    }
    return 0;
}
// 查询登陆表最大序号回调函数
int get_login_maxid_func(void *data, int argc, char **argv, char **azColName)
{
    if (argv[0] != NULL)
    {
        login_maxid = atoi(argv[0]);
    }
    else
    {
        login_maxid = 0;
    }

    return 0;
}
// 查询信息表最大序号
int get_info_max_id(sqlite3 **msgSys_db)
{
    char *sql_insert = NULL;
    char *errMsg = NULL;
    int ret = 0;
    sql_insert = "select MAX(序号) from infoTable;";
    ret = sqlite3_exec(*msgSys_db, sql_insert, get_info_maxid_func, NULL, &errMsg);
    if (SQLITE_OK != ret)
    {
        printf("查询序号失败:%s\n", errMsg);
        return -1;
    }
    return 0;
}
// 查询信息表最大序号回调函数
int get_info_maxid_func(void *data, int argc, char **argv, char **azColName)
{
    if (argv[0] != NULL)
    {
        info_maxid = atoi(argv[0]);
    }
    else
    {
        info_maxid = 0;
    }

    return 0;
}
// 根据姓名查找序号回调函数
int get_id_by_name_func(void *data, int argc, char **argv, char **azColName)
{
    if (argv[0] != NULL)
    {
        *(int *)data = atoi(argv[0]);
    }
    else
    {
        *(int *)data = 0;
    }

    return 0;
}

// 用户名回调函数
int username_function(void *netdata, int argc, char **argv, char **azColName)
{
    int ret;
    netdata_t *data = (netdata_t *)netdata;
    if (strcmp(argv[0], data->login.username) == 0)
    {
        // 用户名正确
        data->privatedata[0] = 1;
    }
    return 0;
}
// 密码回调函数
int password_function(void *netdata, int argc, char **argv, char **azColName)
{
    netdata_t *data = (netdata_t *)netdata;
    if (argv[0] != NULL)
    {
        if (strcmp(argv[0], data->login.password) == 0)
        {
            // 密码正确
            data->login.flags = 1;
        }
        else
        {
            // 密码错误
            data->login.flags = 0;
        }
    }

    return 0;
}

// 根据姓名查找单项回调函数
int serach_by_name_func(void *netdata, int argc, char **argv, char **azColName)
{
    int ret;
    netdata_t *data = (netdata_t *)netdata;
    if (argv[0] != 0)
    {
        // 有数据
        for (int i = 0; i < argc; i++)
        {
            // 把数据库中的数据赋值给netdata
            switch (i)
            {
            case 0:
                data->userinfo.id = atoi(argv[i]);
                break;
            case 1:
                memcpy(data->userinfo.name, argv[i], strlen(argv[i]));
                break;
            case 2:
                memcpy(data->userinfo.sex, argv[i], strlen(argv[i]));
                break;
            case 3:
                memcpy(data->userinfo.age, argv[i], strlen(argv[i]));
                break;
            case 4:
                memcpy(data->userinfo.post, argv[i], strlen(argv[i]));
                break;
            case 5:
                memcpy(data->userinfo.salary, argv[i], strlen(argv[i]));
                break;
            case 6:
                memcpy(data->userinfo.phone, argv[i], strlen(argv[i]));
                break;
            case 7:
                memcpy(data->userinfo.homeaddr, argv[i], strlen(argv[i]));
                break;
            }
        }
        data->privatedata[0] = 1;
    }
    else
    {
        // 没有数据
        data->privatedata[0] = 0;
    }

    return 0;
}

// 全部查找回调函数
int serach_all_func(void *netdata, int argc, char **argv, char **azColName)
{
    int ret, fd;
    char buf[128];
    netdata_t *data = (netdata_t *)netdata;
    memcpy(&fd, data->privatedata, sizeof(int));
    memcpy(data->privatedata + 4, &info_maxid, sizeof(int));
    // snprintf(buf, sizeof(buf), "[%s]\t[%s]\t[%s]\t[%s]\t\t\t[%s]\t\t[%s]\t\t[%s]\t\t\n", azColName[1], azColName[2], azColName[3], azColName[4], azColName[5], azColName[6], azColName[7]);
    memcpy(data->result, "查询成功", sizeof("查询成功"));

    if (argv[0] != NULL)
    {
        // 有数据
        data->userinfo.id = atoi(argv[0]);
        memcpy(data->userinfo.name, argv[1], strlen(argv[1]));
        memcpy(data->userinfo.sex, argv[2], strlen(argv[2]));
        memcpy(data->userinfo.age, argv[3], strlen(argv[3]));
        memcpy(data->userinfo.post, argv[4], strlen(argv[4]));
        memcpy(data->userinfo.salary, argv[5], strlen(argv[5]));
        memcpy(data->userinfo.phone, argv[6], strlen(argv[6]));
        memcpy(data->userinfo.homeaddr, argv[7], strlen(argv[7]));

        ret = send(fd, (netdata_t *)netdata, sizeof(netdata_t), 0);
        if (-1 == ret)
        {
            perror("send error");
            return -1;
        }
    }

    return 0;
}

int login_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata)
{
    char *sql_insert = NULL;
    char *errMsg = NULL;
    int ret = 0;
    netdata.privatedata[0] = 0;
    sql_insert = sqlite3_mprintf("SELECT 用户名 FROM loginTable;");
    ret = sqlite3_exec(*msgSys_db, sql_insert, username_function, &netdata, &errMsg);
    if (ret != SQLITE_OK)
    {
        printf("查找数据失败:%s\n", errMsg);
        return -1;
    }
    // 释放sqlite3在堆上的资源：
    sqlite3_free(sql_insert);
    if (netdata.privatedata[0] == 1)
    {
        // 用户名正确
        sql_insert = sqlite3_mprintf("SELECT 密码 FROM loginTable WHERE 用户名 = '%q';", netdata.login.username);
        ret = sqlite3_exec(*msgSys_db, sql_insert, password_function, &netdata, &errMsg);
        if (ret != SQLITE_OK)
        {
            printf("查找数据失败:%s\n", errMsg);
            return -1;
        }
        // 释放sqlite3在堆上的资源：
        sqlite3_free(sql_insert);
        if (netdata.login.flags == 1)
        {
            // 密码正确
            memcpy(netdata.result, "密码正确", sizeof("密码正确"));
            ret = send(*fd, &netdata, sizeof(netdata), 0);
            if (-1 == ret)
            {
                perror("send error");
                return -1;
            }
        }
        else
        {
            // 密码错误
            memcpy(netdata.result, "密码错误", sizeof("密码错误"));
            ret = send(*fd, &netdata, sizeof(netdata), 0);
            if (-1 == ret)
            {
                perror("send error");
                return -1;
            }
        }
    }
    else if (netdata.privatedata[0] == 0)
    {
        // 用户名错误
        printf("用户名不存在\n");
        memcpy(netdata.result, "用户名不存在", sizeof("用户名不存在"));
        ret = send(*fd, &netdata, sizeof(netdata), 0);
        if (-1 == ret)
        {
            perror("send error");
            return -1;
        }
    }
    return 0;
}
int register_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata)
{
    // 注册
    char *sql_insert = NULL;
    char *errMsg = NULL;
    int ret = 0;
    sql_insert = sqlite3_mprintf("insert into loginTable values ('%d', '%q','%q');",
                                 login_maxid + 1, netdata.login.username, netdata.login.password);
    ret = sqlite3_exec(*msgSys_db, sql_insert, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
    {
        printf("插入数据失败:%s\n", errMsg);
        return -1;
    }
    login_maxid += 1;
    // 释放sqlite3在堆上的资源：
    sqlite3_free(sql_insert);
    memcpy(netdata.result, "注册成功", sizeof("注册成功"));
    ret = send(*fd, &netdata, sizeof(netdata), 0);
    if (-1 == ret)
    {
        perror("send error");
        return -1;
    }
    return 0;
}
int add_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata)
{
    char buf[128] = "正在增加数据,请输入要增加的信息\n";
    memcpy(netdata.result, buf, sizeof(buf));
    int ret = send(*fd, &netdata, sizeof(netdata), 0);
    if (-1 == ret)
    {
        perror("send error");
        return -1;
    }
    return 0;
}
int delete_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata)
{
    char buf[128] = "正在删除数据,请输入要删除信息的姓名\n";
    memcpy(netdata.result, buf, sizeof(buf));
    int ret = send(*fd, &netdata, sizeof(netdata), 0);
    if (-1 == ret)
    {
        perror("send error");
        return -1;
    }
    return 0;
}
int modify_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata)
{
    char *sql_insert = NULL;
    char *errMsg = NULL;
    int ret = 0;
    sql_insert = sqlite3_mprintf("select * from infoTable where 姓名='%q';", netdata.userinfo.name);
    ret = sqlite3_exec(*msgSys_db, sql_insert, serach_by_name_func, &netdata, &errMsg);
    if (ret != SQLITE_OK)
    {
        printf("查找数据失败:%s\n", errMsg);
        return -1;
    }
    // 释放sqlite3在堆上的资源：
    sqlite3_free(sql_insert);
    if (netdata.privatedata[0] == 1)
    {
        // 查询成功
        memcpy(netdata.result, "输入的姓名存在", sizeof("输入的姓名存在"));
        ret = send(*fd, &netdata, sizeof(netdata), 0);
        if (-1 == ret)
        {
            perror("send error");
        }
    }
    else
    {
        // 查询失败
        memcpy(netdata.result, "输入的姓名不存在\n", sizeof("输入的姓名不存在\n"));
        ret = send(*fd, &netdata, sizeof(netdata), 0);
        if (-1 == ret)
        {
            perror("send error");
            return -1;
        }
    }
    return 0;
}
// 子选项
int add_option_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata)
{
    // 添加
    char *sql_insert = NULL;
    char *errMsg = NULL;
    int ret = 0;
    sql_insert = sqlite3_mprintf("insert into infoTable values ('%d', '%q', '%q', '%q', '%q', '%q', '%q', '%q');",
                                 info_maxid + 1, netdata.userinfo.name, netdata.userinfo.sex, netdata.userinfo.age,
                                 netdata.userinfo.post, netdata.userinfo.salary, netdata.userinfo.phone, netdata.userinfo.homeaddr);
    ret = sqlite3_exec(*msgSys_db, sql_insert, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
    {
        printf("插入数据失败:%s\n", errMsg);
        return -1;
    }
    info_maxid += 1;
    // 释放sqlite3在堆上的资源：
    sqlite3_free(sql_insert);
    memcpy(netdata.result, "插入数据成功", sizeof("插入数据成功"));
    ret = send(*fd, &netdata, sizeof(netdata), 0);
    if (-1 == ret)
    {
        perror("send error");
        return -1;
    }
    return 0;
}
int del_option_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata)
{
    // 删除
    char *sql_insert = NULL;
    char *errMsg = NULL;
    int ret = 0;
    int delete_id = 0;
    // 查找要删除的序号
    sql_insert = sqlite3_mprintf("select * from infoTable where 姓名='%q';", netdata.userinfo.name);
    ret = sqlite3_exec(*msgSys_db, sql_insert, get_id_by_name_func, &delete_id, &errMsg);
    if (ret != SQLITE_OK)
    {
        printf("删除数据失败:%s\n", errMsg);
        return -1;
    }
    // 删除这条信息
    sql_insert = sqlite3_mprintf("delete from infoTable where 姓名= '%q';", netdata.userinfo.name);
    ret = sqlite3_exec(*msgSys_db, sql_insert, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
    {
        printf("删除数据失败:%s\n", errMsg);
        return -1;
    }
    // 删除后序号减一
    sql_insert = sqlite3_mprintf("update infoTable set 序号=序号-1 where 序号>'%d';", delete_id);
    ret = sqlite3_exec(*msgSys_db, sql_insert, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
    {
        printf("删除数据失败:%s\n", errMsg);
        return -1;
    }
    info_maxid -= 1;
    // 释放sqlite3在堆上的资源：
    sqlite3_free(sql_insert);

    char buf[20] = "删除数据成功\n";
    memcpy(netdata.result, buf, sizeof(buf));
    ret = send(*fd, &netdata, sizeof(netdata), 0);
    if (-1 == ret)
    {
        perror("send error");
        return -1;
    }
    return 0;
}
int modify_option_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata)
{
    // 修改
    char *sql_insert = NULL;
    char *errMsg = NULL;
    int ret = 0;
    // 修改
    switch (netdata.privatedata[0])
    {
        // 修改性别 // 修改年龄 // 修改职位 // 修改薪水 // 修改电话 // 修改家庭地址
    case 2:
        sql_insert = sqlite3_mprintf("update infoTable set 性别='%q' where 姓名='%q';", netdata.userinfo.sex, netdata.userinfo.name);
        break;
    case 3:
        sql_insert = sqlite3_mprintf("update infoTable set 年龄='%q' where 姓名='%q';", netdata.userinfo.age, netdata.userinfo.name);
        break;
    case 4:
        sql_insert = sqlite3_mprintf("update infoTable set 职位='%q' where 姓名='%q';", netdata.userinfo.post, netdata.userinfo.name);
        break;
    case 5:
        sql_insert = sqlite3_mprintf("update infoTable set 薪水='%q' where 姓名='%q';", netdata.userinfo.salary, netdata.userinfo.name);
        break;
    case 6:
        sql_insert = sqlite3_mprintf("update infoTable set 电话='%q' where 姓名='%q';", netdata.userinfo.phone, netdata.userinfo.name);
        break;
    case 7:
        sql_insert = sqlite3_mprintf("update infoTable set 家庭住址='%q' where 姓名='%q';", netdata.userinfo.homeaddr, netdata.userinfo.name);
        break;
    }
    ret = sqlite3_exec(*msgSys_db, sql_insert, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
    {
        printf("删除数据失败:%s\n", errMsg);
        return -1;
    }
    // 释放sqlite3在堆上的资源：
    sqlite3_free(sql_insert);
    // 修改成功
    memcpy(netdata.result, "修改成功\n", sizeof("修改成功\n"));
    ret = send(*fd, &netdata, sizeof(netdata), 0);
    if (-1 == ret)
    {
        perror("send error");
        return -1;
    }
    return 0;
}
int serach_option_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata)
{
    char *sql_insert = NULL;
    char *errMsg = NULL;
    int ret = 0;
    sql_insert = sqlite3_mprintf("select * from infoTable where 姓名='%q';", netdata.userinfo.name);
    ret = sqlite3_exec(*msgSys_db, sql_insert, serach_by_name_func, &netdata, &errMsg);
    if (ret != SQLITE_OK)
    {
        printf("查找数据失败:%s\n", errMsg);
        return -1;
    }
    // 释放sqlite3在堆上的资源：
    sqlite3_free(sql_insert);
    if (netdata.privatedata[0] == 1)
    {
        // 查询成功
        memcpy(netdata.result, "查询成功", sizeof("查询成功"));
        ret = send(*fd, &netdata, sizeof(netdata), 0);
        if (-1 == ret)
        {
            perror("send error");
        }
    }
    else
    {
        // 查询失败
        memcpy(netdata.result, "查询失败\n", sizeof("查询失败\n"));
        ret = send(*fd, &netdata, sizeof(netdata), 0);
        if (-1 == ret)
        {
            perror("send error");
            return -1;
        }
    }
    return 0;
}

int serach_all_databases(sqlite3 **msgSys_db, int *fd, netdata_t netdata)
{
    char *sql_insert = NULL;
    char *errMsg = NULL;
    int ret = 0;

    memcpy(netdata.privatedata, &(*fd), sizeof(int));
    sql_insert = sqlite3_mprintf("select * from infoTable;");
    ret = sqlite3_exec(*msgSys_db, sql_insert, serach_all_func, &netdata, &errMsg);
    if (ret != SQLITE_OK)
    {
        printf("查找数据失败:%s\n", errMsg);
        return -1;
    }
    // 释放sqlite3在堆上的资源：
    sqlite3_free(sql_insert);
    return 0;
}