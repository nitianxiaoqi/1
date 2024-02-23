#include "project.h"

// 网络初始化
int Net_init(const char *IP, const char *PORT)
{
    int listen_fd = 0;

    // 申请套接字
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        return -1;
    }
    // 绑定地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port = htons(atoi(PORT));

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(listen_fd, MAX_CLIENTS) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // 返回文件描述符
    return listen_fd;
}

// 消息队列初始化
int msgq_init(void)
{
    // 消息队列初始化
    key = ftok(MSGPATH, 'm');
    if (-1 == key)
    {
        return -1;
    }

    msqid = msgget(key, IPC_CREAT | 0666);
    if (-1 == msqid)
    {
        return -1;
    }

    return 0;
}

// 链表初始化
Node_t *linklist_init(void)
{
    Node_t *L = (Node_t *)malloc(sizeof(Node_t));
    if (NULL == L)
    {
        return L;
    }

    memset(L, 0, sizeof(Node_t));
    L->next = NULL;

    return L;
}

// 头部插入
int hendinsert(Node_t *L, const char *username, int fd)
{
    if (NULL == L)
    {
        return -1;
    }

    Node_t *p = linklist_init();

    strcpy(p->name, username);
    p->fd = fd;

    p->next = L->next;
    L->next = p;

    return 0;
}

// 根据ID查找
int namefind(Node_t *L, const char *username)
{
    if (NULL == L || NULL == L->next)
    {
        return -1;
    }

    Node_t *q = L->next;

    while (q)
    {
        if (!strcmp(q->name, username))
        {
            return 0;
        }
        q = q->next;
    }

    return -1;
}

// 根据文件描述符删除
int filedelete(Node_t *L, int fd)
{
    if (NULL == L || NULL == L->next)
    {
        return -1;
    }

    Node_t *q = L;
    Node_t *p = NULL;
    while (q->next)
    {
        if (fd == q->next->fd)
        {
            p = q->next;
            q->next = p->next;
            p->next = NULL;
            free(p);
            return 0;
        }
        q = q->next;
    }

    return -1;
}
// 遍历链表
void showlist(Node_t *L)
{
    Node_t *q = L->next;
    puts("当前在线用户有：");
    while (q)
    {
        printf("%s:%d\n", q->name, q->fd);
        q = q->next;
    }
}
// 释放链表
int freelist(Node_t **L)
{
    if (NULL == L || NULL == *L)
    {
        return -1;
    }

    Node_t *p, *q;
    q = *L;
    while (q)
    {
        p = q;
        q = q->next;
        free(p);
    }

    *L = NULL;
    return 0;
}

// 上位机检索下位机线程
void *handler_uprequest(void *argv)
{
    Msg_t buf;
    long retmsgvalue;
    while (1)
    {
        //  监控消息队列中的 类型为 1  消息
        memset(&buf, 0, sizeof(Msg_t));
        msgrcv(msqid, &buf, sizeof(Msg_t) - sizeof(long), 1, 0);
        printf("收到上位机%s登录请求\n", buf.userinfo);
        // 拿到消息数据中的 userinfo 数据  并获取cgi等待的消息类型数据 recvtype
        // 字段的值
        retmsgvalue = buf.recvtype;
        printf("buf.recvtype = %ld\n", retmsgvalue);
        // 在链表中遍历 此节点是否存在
        // 存在 将 msg 结构中 flags 字段 置 1并发送消息 recvtype 再赋值 发送消息
        // 不存在 flags 置 0 与存在同理

        // 互斥机制
        if (!namefind(L, buf.userinfo))
        {
            puts("下位机存在");
            memset(&buf, 0, sizeof(Msg_t));
            buf.flags = 1;
            buf.msgtype = retmsgvalue;
            msgsnd(msqid, &buf, sizeof(Msg_t) - sizeof(long), 0);
        }
        else
        {
            puts("下位机不存在");
            memset(&buf, 0, sizeof(Msg_t));
            buf.flags = 0;
            buf.msgtype = retmsgvalue;
            msgsnd(msqid, &buf, sizeof(Msg_t) - sizeof(long), 0);
        }
    }
}
int username_function(void *loginInfo, int argc, char **argv, char **azColName);
int password_function(void *loginInfo, int argc, char **argv, char **azColName);
// 下位机处理线程(有下位机连接，就把它加入链表)
void *handler_downrequest(void *argv)
{
    int fd = *((int *)argv);
    Netdata_t buf;

    int ret;
    sqlite3 *iotSys_db = NULL;
    char *errmsg = NULL;
    char *sql = NULL;

    long usermsgtype = 0;
    char *s = NULL;

    // 等待登录数据
    if (0 == recv(fd, &buf, sizeof(Netdata_t), 0))
    {
        close(fd);
        pthread_exit(NULL);
    }
    // 判断数据
    memcpy(loginInfo.username, buf.ID, sizeof(buf.ID));
    memcpy(loginInfo.password, buf.PS, sizeof(buf.PS));
    // 创建一个数据库实例（数据库的文件）并打开。
    ret = sqlite3_open("../database/iotSysTcp.db", &iotSys_db);
    if (ret != SQLITE_OK)
    {
        printf("数据库打开异常\n");
    }
    // 创建数据表：
    sql = "create table if not exists loginTable(序号 int ,用户名 text int primary key, 密码 text);";
    ret = sqlite3_exec(iotSys_db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("执行sql失败:%s\n", errmsg);
    }

    loginInfo.flags = 0;
    sql = sqlite3_mprintf("SELECT 用户名 FROM loginTable;");
    ret = sqlite3_exec(iotSys_db, sql, username_function, &loginInfo, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("查找数据失败:%s\n", errmsg);
    }
    if (loginInfo.flags == 1)
    {
        // 用户名正确
        sql = sqlite3_mprintf("SELECT 密码 FROM loginTable WHERE 用户名 = '%q';", loginInfo.username);
        ret = sqlite3_exec(iotSys_db, sql, password_function, &loginInfo, &errmsg);
        if (ret != SQLITE_OK)
        {
            printf("查找数据失败:%s\n", errmsg);
        }
        if (loginInfo.flags == 1)
        {
            // 密码正确
            // 正确 封装链表节点 头插链表
            printf("%s:登陆成功\n", buf.ID);
            s = buf.ID;
            while (*s)
            {
                usermsgtype += *s;
                s++;
            }
            buf.flags = 1;
            send(fd, &buf, sizeof(Netdata_t), 0);
            // 互斥机制
            hendinsert(L, buf.ID, fd);
            showlist(L);

            while (1)
            {
                // 监控消息队列
                printf("等待上位机消息,消息类型：%ld\n", usermsgtype);
                memset(&buf, 0, sizeof(Netdata_t));
                msgrcv(msqid, &(buf.msg), sizeof(Msg_t) - sizeof(long), usermsgtype, 0);
                printf("---------------comd:%d\n", buf.msg.comd);
                printf("Netdata_t : %ld\n", sizeof(Netdata_t));
                // 将消息队列数据通过socket发送给智能硬件
                send(fd, &buf, sizeof(Netdata_t), 0);
                memset(&buf, 0, sizeof(Netdata_t));
                printf("temup= %f\n", buf.msg.limit.tempup);
                printf("temdown= %f\n", buf.msg.limit.tempdown);
                printf("消息已经发送给下位机\n");
                // 等待下位机返回执行结果
                if (0 == recv(fd, &buf, sizeof(Netdata_t), 0))
                {
                    close(fd);
                    // 互斥机制
                    filedelete(L, fd);
                    buf.msg.msgtype = usermsgtype * 2;
                    buf.msg.flags = 0; // 下位机离线
                    msgsnd(msqid, &(buf.msg), sizeof(Msg_t) - sizeof(long), 0);
                    pthread_exit(NULL);
                }
                printf("已经接受到客户端发来的消息\n");
                // 将执行结果数据回写到消息队列
                buf.msg.msgtype = usermsgtype * 2;
                buf.msg.flags = 1;
                msgsnd(msqid, &(buf.msg), sizeof(Msg_t) - sizeof(long), 0);
                printf("消息类型*2:%ld,返回给boa\n", usermsgtype * 2);
            }
            return 0;
        }
        else if (loginInfo.flags == 0)
        {
            // 密码错误
            // 错误 释放资源 线程退出
            buf.flags = 0;
            send(fd, &buf, sizeof(Netdata_t), 0);
            pthread_exit(NULL);
        }
    }
    else if (loginInfo.flags == 0)
    {
        // 用户名错误
        // 错误 释放资源 线程退出
        buf.flags = 0;
        send(fd, &buf, sizeof(Netdata_t), 0);
        pthread_exit(NULL);
    }
}

// 用户名回调函数
int username_function(void *loginInfo, int argc, char **argv, char **azColName)
{
    int ret;
    loginInfo_t *data = (loginInfo_t *)loginInfo;
    if (strncmp(argv[0], data->username, strlen(argv[0])) == 0)
    {
        // 用户名正确
        data->flags = 1;
    }
    return 0;
}
// 密码回调函数
int password_function(void *loginInfo, int argc, char **argv, char **azColName)
{
    loginInfo_t *data = (loginInfo_t *)loginInfo;
    if (argv[0] != NULL)
    {
        if (strncmp(argv[0], data->password, strlen(argv[0])) == 0)
        {
            // 密码正确
            data->flags = 1;
        }
        else
        {
            // 密码错误
            data->flags = 0;
        }
    }
    return 0;
}