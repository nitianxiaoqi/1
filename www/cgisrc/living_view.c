#include "project.h"

// alarm信号处理函数
void sighandler(int argc)
{
    msgrcv(msqid, &msg, sizeof(Msg_t) - sizeof(long), mrecv, IPC_NOWAIT);
    msgrcv(msqid, &msg, sizeof(Msg_t) - sizeof(long), mrecv * 2, IPC_NOWAIT);
    printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
    printf("<!DOCTYPE html>");
    printf("<html>");
    printf("<body>");
    printf("mrecv = %ld", mrecv);
    printf("cookvalue = %s", cookvalue);
    printf("msg.userinfo = %s", msg.userinfo);
    printf("<h2>网络原因导致数据不可达，请稍后刷新重试</h2>");
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
    msg.msgtype = mrecv;
    msg.comd = 1; // 查看环境请求
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

    if (msg.flags)
    {
        printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
        printf("<!DOCTYPE html>");
        printf("<html>");
        printf("<body>");
        printf("<center>");
        printf("<table  border = \"5\">");
        printf("<th bgcolor=\"#BEBEBE\" width=\"80\" height=\"20\"> UserID</th>");
        printf("<tr>");
        printf("<td bgcolor=\"#00BFFF\">%s</td>", loginInfo.username);
        printf("</tr>");
        printf("</table>");
        printf("<h3>环境数据</h3>");
        printf("<table  border = \"3\">");
        printf("<th bgcolor=\"#BEBEBE\" width=\"80\" height=\"20\"> 温度 </th>");
        printf("<th bgcolor=\"#BEBEBE\" width=\"80\" height=\"20\"> 湿度 </th>");
        printf("<th bgcolor=\"#BEBEBE\" width=\"80\" height=\"20\"> 光强 </th>");
        printf("<th bgcolor=\"#BEBEBE\" width=\"80\" height=\"20\"> 电压 </th>");
        printf("<tr>");
        printf("<td bgcolor=\"#00BFFF\">%.2f</td>", msg.env.tempvalue);
        printf("<td bgcolor=\"#00BFFF\">%d%%</td>", msg.env.humevalue);
        printf("<td bgcolor=\"#00BFFF\">%hd</td>", msg.env.uillevalue);
        printf("<td bgcolor=\"#00BFFF\">%.2fv</td>", msg.env.volvalue);
        printf("</tr>");
        printf("</table>");

        printf("<h3>设备状态</h3>");
        printf("<table  border = \"3\">");
        printf("<th bgcolor=\"#BEBEBE\" width=\"80\" height=\"20\"> 照明 </th>");
        printf("<th bgcolor=\"#BEBEBE\" width=\"80\" height=\"20\"> 温控 </th>");
        printf("<th bgcolor=\"#BEBEBE\" width=\"80\" height=\"20\"> 通风 </th>");
        printf("<th bgcolor=\"#BEBEBE\" width=\"80\" height=\"20\"> 警报 </th>");
        printf("<tr>");
        if (msg.env.ledstate)
        {
            printf("<td bgcolor=\"#00BFFF\" width=\"80\" height=\"20\">开</td>");
        }
        else
        {
            printf("<td bgcolor=\"#00BFFF\" width=\"80\" height=\"20\">关</td>");
        }
        if (msg.env.fanstate)
        {
            printf("<td bgcolor=\"#00BFFF\" width=\"80\" height=\"20\">开</td>");
        }
        else
        {
            printf("<td bgcolor=\"#00BFFF\" width=\"80\" height=\"20\">关</td>");
        }
        if (msg.env.humestate)
        {
            printf("<td bgcolor=\"#00BFFF\" width=\"80\" height=\"20\">开</td>");
        }
        else
        {
            printf("<td bgcolor=\"#00BFFF\" width=\"80\" height=\"20\">关</td>");
        }
        if (msg.env.beepstate)
        {
            printf("<td bgcolor=\"#00BFFF\" width=\"80\" height=\"20\">开</td>");
        }
        else
        {
            printf("<td bgcolor=\"#00BFFF\" width=\"80\" height=\"20\">关</td>");
        }
        printf("</tr>");
        printf("</table>");
        printf("<form action=\"../cgi-bin/living_view.cgi\" method=\"get\">");
        printf("<input type=\"submit\" name=\"login_button\" value=\"点击获取环境数据\" />");
        printf("</form>");
        printf("</center>");
        printf("</body>");
        printf("</html>");
    }
    else
    {
        printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
        printf("<!DOCTYPE html>");
        printf("<html>");
        printf("<body>");
        printf("<center>");
        printf("mrecv = %ld", mrecv);
        printf("cookvalue = %s", cookvalue);
        printf("msg.userinfo = %s", msg.userinfo);
        printf("<h2>网络原因导致数据不可达，请稍后刷新重试<h2>");
        printf("</center>");
        printf("</body>");
        printf("</html>");
    }
    return 0;
}
