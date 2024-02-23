#include "project.h"
loginInfo_t loginInfo;
// alarm信号处理函数
void sig_handler(int argc)
{
    // printf("Content-type: text/html;charset=\"UTF-8\"\n\n");
    // printf("Set-Cookie:username=%s;path=/;", loginInfo.username);

    while (-1 != msgrcv(msqid, &msg, sizeof(Msg_t) - sizeof(long), 1, IPC_NOWAIT))
    {
        if (msg.recvtype == mrecv * 2)
        {
            printf("Content-type: text/html;charset=\"UTF-8\"\n\n");
            printf("<!DOCTYPE html>");
            printf("<html>");
            printf("<body>");
            printf("<h2>cookie:%s网络原因导致数据不可达，请稍后重试，或联系管理员。电话：xxxxx</h2>", cookvalue);
            printf("<a href=\"../index.html\" target=\"blank\"><h1>返回登录</h1></a>");
            printf("</body>");
            printf("</html>");
            exit(-1);
        }
        else
        {
            msgsnd(msqid, &msg, sizeof(Msg_t) - sizeof(long), 0);
        }
    }
}

int password_function(void *loginInfo, int argc, char **argv, char **azColName);

int cgiMain(int argc, const char *argv[])
{
    int ret;
    sqlite3 *iotSys_db = NULL;
    char *errmsg = NULL;
    char *sql = NULL;

    // 信号初始化
    signal(SIGALRM, sig_handler);
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
    // 登录信息结构体初始化
    memset(&loginInfo, 0, sizeof(loginInfo_t));

    // 获取网页用户名和密码数据
    cgiFormString("ID", loginInfo.username, 20);
    cgiFormString("PASSWORD", loginInfo.password, 7);

    memcpy(cookvalue, cgiCookie, 128);

    // 创建一个数据库实例（数据库的文件）并打开。
    ret = sqlite3_open("../database/iotSys.db", &iotSys_db);
    if (ret != SQLITE_OK)
    {
        printf("数据库打开异常\n");
        return -1;
    }
    // 创建数据表：
    sql = "create table if not exists loginTable(序号 int ,用户名 text int primary key, 密码 text);";
    ret = sqlite3_exec(iotSys_db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("执行sql失败:%s\n", errmsg);
        return -1;
    }

    sql = sqlite3_mprintf("SELECT 密码 FROM loginTable WHERE 用户名 = '%q';", loginInfo.username);
    ret = sqlite3_exec(iotSys_db, sql, password_function, NULL, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("查找数据失败:%s\n", errmsg);
        return -1;
    }
    if (loginInfo.flags == 1)
    {
        // 密码正确
        // 发送消息
        memset(&msg, 0, sizeof(Msg_t));

        strcpy(msg.userinfo, loginInfo.username);
        mrecv = 0;
        char *s = msg.userinfo;
        while (*s)
        {
            mrecv += *s;
            s++;
        }
        msg.msgtype = 1;
        msg.recvtype = mrecv * 2;
        if (-1 == msgsnd(msqid, &msg, sizeof(Msg_t) - sizeof(long), 0))
        {
            return -1;
        }
        // 设置5秒超时
        alarm(5);
        // 读取消息 等待结果
        memset(&msg, 0, sizeof(Msg_t));
        if (-1 == msgrcv(msqid, &msg, sizeof(Msg_t) - sizeof(long), mrecv * 2, 0))
        {
            return -1;
        }
        alarm(5);

        if (msg.flags)
        {
            printf("Set cookie:username=%s;path=/;", loginInfo.username);
            printf("Content-type: text/html;charset=\"UTF-8\"\n\n");
            printf("<!DOCTYPE html>");
            printf("<html>");
            printf("<body>");
            printf("<script>window.location.href = '../Iot_select.html';</script>");
            printf("</body>");
            printf("</html>");
            return 0;
        }
        printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
        printf("<!DOCTYPE html>");
        printf("<html>");
        printf("<body>");
        printf("<center>");
        printf("<h3>登录失败，原因为下位机未登录，请将下位机联网或联系 管理员二狗  电话：123456131</h3>");
        printf("<a href=\"../index.html\" target=\"blank\"><h1>返回登录</h1></a>");
        printf("</center>");
        printf("</body>");
        printf("</html>");

        return 0;
    }
    else if (loginInfo.flags == 0)
    {
        // 密码错误
        printf("<center>");
        printf("cookvalue:%s\n", cookvalue);
        printf("cgiCookie:%s\n", cgiCookie);
        printf("<h1>密码错误</h1><br>");
        printf("<a href=\"../index.html\" target=\"blank\"><h1>返回登录</h1></a>");
        printf("</center>");
    }

    // 释放sqlite3在堆上的资源：
    sqlite3_free(sql);
    return 0;
}

// 密码回调函数
int password_function(void *data, int argc, char **argv, char **azColName)
{
    if (argv[0] != NULL)
    {
        if (strncmp(argv[0], loginInfo.password, strlen(argv[0])) == 0)
        {
            // 密码正确
            loginInfo.flags = 1;
        }
        else
        {
            // 密码错误
            loginInfo.flags = 0;
        }
    }
    return 0;
}