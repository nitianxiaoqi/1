#include "project.h"

char name[20] = {0};	// 存储网页用户名的缓冲区
char password[7] = {0}; // 存储网页密码的缓冲区

char ID[] = "cxy"; // 案例参考值
char PS[] = "123456";

// alarm信号处理函数
void sighandler(int argc)
{

	// printf("Set-Cookie:username=%s;path=/;",name);
	while (-1 != msgrcv(msqid, &msg, sizeof(Msg_t) - sizeof(long), 1, IPC_NOWAIT))
	{
		if (msg.recvtype == mrecv * 2)
		{
			printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
			printf("<!DOCTYPE html>");
			printf("<html>");
			printf("<body>");
			printf("<h2>cookie:%s网络原因导致数据不可达，请稍后重试，或联系管理员。电话：xxxxxx</h2>", cookvalue);
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

	cgiFormString("ID", name, 20);			// 获取网页用户名数据
	cgiFormString("PASSWORD", password, 7); // 获取网页密码数据

	memcpy(cookvalue, cgiCookie, 128);

	if (0 == strcmp(ID, name) && 0 == strcmp(PS, password))
	{ // 比较  成功

		// 发送消息
		memset(&msg, 0, sizeof(Msg_t));

		strcpy(msg.userinfo, name);
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
			printf("Set-Cookie:username=%s;path=/;", name);
			printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
			printf("<!DOCTYPE html>");
			printf("<html>");
			printf("<body>");
			printf("<script>window.location.href='../Iot_select.html';</script>");
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
	else
	{
		printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
		printf("<!DOCTYPE html>");
		printf("<html>");
		printf("<body>");
		printf("<center>");
		printf("name = %s\n", name);
		printf("ID = %s\n", ID);
		printf("password = %s\n", password);
		printf("PS = %s\n", PS);
		printf("<h3>登录失败，原因可能是用户名或密码错误导致。也有可能是您本身未注册系统用户，请联系 管理员二狗  电话：123456131</h3>");
		printf("<a href=\"../index.html\" target=\"blank\"><h1>返回登录</h1></a>");
		printf("</center>");
		printf("</body>");
		printf("</html>");
	}

	return 0;
}
