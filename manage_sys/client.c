#include <my_head.h>
#define PORT 8888

// 定义颜色码
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

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

int send_recv_func(int server_fd, netdata_t *sent_netdata, netdata_t *recv_netdata);

int main(int argc, const char *argv[])
{
    char ip_addr[20] = {0};
    int port = 0;
    char buf[512] = {0};
    int nbytes = 0;
    netdata_t sent_netdata = {0};
    netdata_t recv_netdata = {0};
    int choice1 = 0, choice2 = 0, choice3 = 0, choice4 = 0;
    int modify_option = 0;
    // socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == server_fd)
    {
        perror("socket error");
        return -1;
    }
    // connect
    struct sockaddr_in server_info =
        {
            .sin_family = AF_INET,
            .sin_addr.s_addr = inet_addr("192.168.250.100"),
            .sin_port = htons(PORT),
        };
    int server_len = sizeof(server_info);
    while (true)
    {
        // 第一级菜单
        printf(ANSI_COLOR_CYAN "=== 联网窗口 ===\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_MAGENTA "1. 自选联网\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_MAGENTA "2. 闲置\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_MAGENTA "3. 退出\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_CYAN "请输入您的选择：" ANSI_COLOR_RESET);
        scanf("%d", &choice1);

        if (choice1 == 1)
        {
            printf(ANSI_COLOR_GREEN "请输入ip地址:" ANSI_COLOR_RESET);
            scanf("%s", ip_addr);
            server_info.sin_addr.s_addr = inet_addr(ip_addr);
            printf(ANSI_COLOR_GREEN "请输入端口号:" ANSI_COLOR_RESET);
            scanf("%d", &port);
            server_info.sin_port = htons(port);

            int ret = connect(server_fd, (const struct sockaddr *)&server_info, server_len);
            if (-1 == ret)
            {
                perror("connect error");
                return -1;
            }
            while (1)
            {
                // 第二级菜单
                printf(ANSI_COLOR_CYAN "=== 登录窗口 ===\n" ANSI_COLOR_RESET);
                printf(ANSI_COLOR_MAGENTA "1. 登录\n" ANSI_COLOR_RESET);
                printf(ANSI_COLOR_MAGENTA "2. 注册\n" ANSI_COLOR_RESET);
                printf(ANSI_COLOR_MAGENTA "3. 返回\n" ANSI_COLOR_RESET);
                printf(ANSI_COLOR_CYAN "请输入您的选择：" ANSI_COLOR_RESET);
                scanf("%d", &choice2);

                if (choice2 == 1)
                {
                    printf(ANSI_COLOR_GREEN "(登录)请输入用户名:" ANSI_COLOR_RESET);
                    scanf("%s", sent_netdata.login.username);
                    printf(ANSI_COLOR_GREEN "(登录)请输入密码:" ANSI_COLOR_RESET);
                    scanf("%s", sent_netdata.login.password);

                    sent_netdata.command = CMD_LOGIN;
                    ret = send_recv_func(server_fd, &sent_netdata, &recv_netdata);
                    if (-1 == ret)
                    {
                        perror("send_recv_func error");
                        return -1;
                    }
                    if (recv_netdata.login.flags == 1)
                    {
                        while (1)
                        {
                            // 第三级菜单
                            printf(ANSI_COLOR_CYAN "=== 选择窗口 ===\n" ANSI_COLOR_RESET);
                            printf(ANSI_COLOR_MAGENTA "1. 增加数据\n" ANSI_COLOR_RESET);
                            printf(ANSI_COLOR_MAGENTA "2. 删除数据\n" ANSI_COLOR_RESET);
                            printf(ANSI_COLOR_MAGENTA "3. 修改数据\n" ANSI_COLOR_RESET);
                            printf(ANSI_COLOR_MAGENTA "4. 查找数据\n" ANSI_COLOR_RESET);
                            printf(ANSI_COLOR_MAGENTA "5. 返回上级菜单\n" ANSI_COLOR_RESET);
                            printf(ANSI_COLOR_MAGENTA "6. 退出\n" ANSI_COLOR_RESET);
                            printf(ANSI_COLOR_CYAN "请输入您的选择：" ANSI_COLOR_RESET);
                            scanf("%d", &choice3);

                            if (choice3 == 1)
                            {
                                sent_netdata.command = CMD_ADD;
                                ret = send_recv_func(server_fd, &sent_netdata, &recv_netdata);
                                if (-1 == ret)
                                {
                                    perror("send_recv_func error");
                                    return -1;
                                }

                                while (true)
                                {
                                    // 第四级菜单
                                    printf(ANSI_COLOR_CYAN "=== 增加窗口 ===\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_GREEN "请输入姓名：" ANSI_COLOR_RESET);
                                    scanf("%s", sent_netdata.userinfo.name);
                                    printf(ANSI_COLOR_GREEN "请输入性别：" ANSI_COLOR_RESET);
                                    scanf("%s", sent_netdata.userinfo.sex);
                                    printf(ANSI_COLOR_GREEN "请输入年龄：" ANSI_COLOR_RESET);
                                    scanf("%s", sent_netdata.userinfo.age);
                                    printf(ANSI_COLOR_GREEN "请输入岗位：" ANSI_COLOR_RESET);
                                    scanf("%s", sent_netdata.userinfo.post);
                                    printf(ANSI_COLOR_GREEN "请输入工资：" ANSI_COLOR_RESET);
                                    scanf("%s", sent_netdata.userinfo.salary);
                                    printf(ANSI_COLOR_GREEN "请输入电话：" ANSI_COLOR_RESET);
                                    scanf("%s", sent_netdata.userinfo.phone);
                                    printf(ANSI_COLOR_GREEN "请输入家庭住址：" ANSI_COLOR_RESET);
                                    scanf("%s", sent_netdata.userinfo.homeaddr);

                                    printf(ANSI_COLOR_MAGENTA "1. 确定增加\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_MAGENTA "2. 返回上级菜单\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_CYAN "请输入您的选择：" ANSI_COLOR_RESET);
                                    scanf("%d", &choice4);

                                    if (choice4 == 1)
                                    {
                                        sent_netdata.command = CMD_ADD_OPTION;
                                        ret = send_recv_func(server_fd, &sent_netdata, &recv_netdata);
                                        if (-1 == ret)
                                        {
                                            perror("send_recv_func error");
                                            return -1;
                                        }
                                        break;
                                    }
                                    else if (choice4 == 2)
                                    {
                                        break; // 返回上级菜单
                                    }
                                    else
                                    {
                                        printf("无效的选择，请重新输入\n");
                                    }
                                }
                            }
                            else if (choice3 == 2)
                            {
                                sent_netdata.command = CMD_DEL;
                                ret = send_recv_func(server_fd, &sent_netdata, &recv_netdata);
                                if (-1 == ret)
                                {
                                    perror("send_recv_func error");
                                    return -1;
                                }
                                while (true)
                                {
                                    // 第四级菜单
                                    printf(ANSI_COLOR_CYAN "=== 删除窗口 ===\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_GREEN "请输入要删除信息的姓名：" ANSI_COLOR_RESET);
                                    scanf("%s", sent_netdata.userinfo.name);

                                    printf(ANSI_COLOR_MAGENTA "1. 确定删除\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_MAGENTA "2. 返回上级菜单\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_CYAN "请输入您的选择：" ANSI_COLOR_RESET);
                                    scanf("%d", &choice4);

                                    if (choice4 == 1)
                                    {
                                        sent_netdata.command = CMD_DEL_OPTION;
                                        ret = send_recv_func(server_fd, &sent_netdata, &recv_netdata);
                                        if (-1 == ret)
                                        {
                                            perror("send_recv_func error");
                                            return -1;
                                        }
                                        break;
                                    }
                                    else if (choice4 == 2)
                                    {
                                        break; // 返回上级菜单
                                    }
                                    else
                                    {
                                        printf("无效的选择，请重新输入\n");
                                    }
                                }
                            }
                            else if (choice3 == 3)
                            {
                                while (true)
                                {
                                    // 第四级菜单
                                    printf(ANSI_COLOR_CYAN "=== 修改窗口 ===\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_GREEN "请输入要修改信息的姓名：" ANSI_COLOR_RESET);
                                    scanf("%s", sent_netdata.userinfo.name);

                                    printf(ANSI_COLOR_MAGENTA "1. 确定修改\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_MAGENTA "2. 返回上级菜单\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_CYAN "请输入您的选择：" ANSI_COLOR_RESET);
                                    scanf("%d", &choice4);

                                    if (choice4 == 1)
                                    {
                                        sent_netdata.command = CMD_MODIFY;
                                        ret = send_recv_func(server_fd, &sent_netdata, &recv_netdata);
                                        if (-1 == ret)
                                        {
                                            perror("send_recv_func error");
                                            return -1;
                                        }
                                        if (strcmp(recv_netdata.result, "输入的姓名不存在") == 0)
                                        {
                                            modify_option = 0;
                                            break;
                                        }
                                        else if (strcmp(recv_netdata.result, "修改成功") == 0)
                                        {
                                            printf(ANSI_COLOR_MAGENTA "2.性别 3.年龄 4.岗位 5.工资 6.电话 7.家庭住址\n" ANSI_COLOR_RESET);
                                            printf(ANSI_COLOR_GREEN "请选择要修改哪一项：" ANSI_COLOR_RESET);
                                            scanf("%d", &modify_option);
                                            switch (modify_option)
                                            {
                                            case 2:
                                                printf(ANSI_COLOR_GREEN "请输入要修改的性别：" ANSI_COLOR_RESET);
                                                scanf("%s", sent_netdata.userinfo.sex);
                                                sent_netdata.privatedata[0] = 2;
                                                break;
                                            case 3:
                                                printf(ANSI_COLOR_GREEN "请输入要修改的年龄：" ANSI_COLOR_RESET);
                                                scanf("%s", sent_netdata.userinfo.age);
                                                sent_netdata.privatedata[0] = 3;
                                                break;
                                            case 4:
                                                printf(ANSI_COLOR_GREEN "请输入要修改的岗位：" ANSI_COLOR_RESET);
                                                scanf("%s", sent_netdata.userinfo.post);
                                                sent_netdata.privatedata[0] = 4;
                                                break;
                                            case 5:
                                                printf(ANSI_COLOR_GREEN "请输入要修改的工资：" ANSI_COLOR_RESET);
                                                scanf("%s", sent_netdata.userinfo.salary);
                                                sent_netdata.privatedata[0] = 5;
                                                break;
                                            case 6:
                                                printf(ANSI_COLOR_GREEN "请输入要修改的电话：" ANSI_COLOR_RESET);
                                                scanf("%s", sent_netdata.userinfo.phone);
                                                sent_netdata.privatedata[0] = 6;
                                                break;
                                            case 7:
                                                printf(ANSI_COLOR_GREEN "请输入要修改的家庭住址：" ANSI_COLOR_RESET);
                                                scanf("%s", sent_netdata.userinfo.homeaddr);
                                                sent_netdata.privatedata[0] = 7;
                                                break;
                                            }
                                            sent_netdata.command = CMD_MODIFY_OPTION;
                                            ret = send_recv_func(server_fd, &sent_netdata, &recv_netdata);
                                            if (-1 == ret)
                                            {
                                                perror("send_recv_func error");
                                                return -1;
                                            }
                                        }
                                        modify_option = 0;
                                        break;
                                    }
                                    else if (choice4 == 2)
                                    {
                                        break; // 返回上级菜单
                                    }
                                    else
                                    {
                                        printf("无效的选择，请重新输入\n");
                                    }
                                }
                            }
                            else if (choice3 == 4)
                            {
                                while (true)
                                {
                                    // 第四级菜单
                                    printf(ANSI_COLOR_CYAN "=== 查询窗口 ===\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_MAGENTA "1. 根据姓名单项查询\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_MAGENTA "2. 查询全部\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_MAGENTA "3. 返回上级菜单\n" ANSI_COLOR_RESET);
                                    printf(ANSI_COLOR_CYAN "请输入您的选择：" ANSI_COLOR_RESET);
                                    scanf("%d", &choice4);

                                    if (choice4 == 1)
                                    {
                                        printf(ANSI_COLOR_GREEN "请输入要查询的姓名：" ANSI_COLOR_RESET);
                                        scanf("%s", sent_netdata.userinfo.name);
                                        int ret;
                                        sent_netdata.command = CMD_SERACH_OPTION;
                                        ret = send(server_fd, &sent_netdata, sizeof(netdata_t), 0);
                                        if (-1 == ret)
                                        {
                                            perror("send error");
                                            return -1;
                                        }
                                        ret = recv(server_fd, &recv_netdata, sizeof(netdata_t), 0);
                                        if (-1 == ret)
                                        {
                                            perror("recv error");
                                            return -1;
                                        }
                                        if (0 == ret)
                                        {
                                            printf("服务器关闭\n");
                                            return -1;
                                        }
                                        printf("%s\n", recv_netdata.result);
                                        if (0 == strcmp(recv_netdata.result, "查询成功"))
                                        {
                                            printf("[序号]\t[姓名]\t[性别]\t[年龄]\t[岗位]\t\t[工资]\t[电话]\t\t[家庭住址]\t\t\n");
                                            printf("[%d]\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\t\t\n", recv_netdata.userinfo.id, recv_netdata.userinfo.name, recv_netdata.userinfo.sex, recv_netdata.userinfo.age, recv_netdata.userinfo.post, recv_netdata.userinfo.salary, recv_netdata.userinfo.phone, recv_netdata.userinfo.homeaddr);
                                        }
                                        // 清空send_netdata、recv_netdata结构体
                                        memset(&sent_netdata, 0, sizeof(netdata_t));
                                        break;
                                    }
                                    else if (choice4 == 2)
                                    {
                                        sent_netdata.command = CMD_SERACH_ALL;
                                        ret = send(server_fd, &sent_netdata, sizeof(netdata_t), 0);
                                        if (-1 == ret)
                                        {
                                            perror("send error");
                                            return -1;
                                        }
                                        int num;
                                        memset(&recv_netdata, 0, sizeof(netdata_t));
                                        ret = recv(server_fd, &recv_netdata, sizeof(netdata_t), 0);
                                        if (-1 == ret)
                                        {
                                            perror("recv error");
                                            return -1;
                                        }
                                        if (0 == ret)
                                        {
                                            printf("服务器关闭\n");
                                            return -1;
                                        }
                                        memcpy(&num, recv_netdata.privatedata + 4, sizeof(int));
                                        printf("%s\n", recv_netdata.result);
                                        printf("[序号]\t[姓名]\t[性别]\t[年龄]\t[岗位]\t\t[工资]\t[电话]\t\t[家庭住址]\t\t\n");
                                        printf("[%d]\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\t\t\n", recv_netdata.userinfo.id, recv_netdata.userinfo.name, recv_netdata.userinfo.sex, recv_netdata.userinfo.age, recv_netdata.userinfo.post, recv_netdata.userinfo.salary, recv_netdata.userinfo.phone, recv_netdata.userinfo.homeaddr);
                                        for (int i = 0; i < num - 1; i++)
                                        {
                                            ret = recv(server_fd, &recv_netdata, sizeof(netdata_t), 0);
                                            if (-1 == ret)
                                            {
                                                perror("recv error");
                                                return -1;
                                            }
                                            if (0 == ret)
                                            {
                                                printf("服务器关闭\n");
                                                return -1;
                                            }
                                            printf("[%d]\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\t\t\n", recv_netdata.userinfo.id, recv_netdata.userinfo.name, recv_netdata.userinfo.sex, recv_netdata.userinfo.age, recv_netdata.userinfo.post, recv_netdata.userinfo.salary, recv_netdata.userinfo.phone, recv_netdata.userinfo.homeaddr);
                                        }
                                        break;
                                    }
                                    else if (choice4 == 3)
                                    {
                                        break; // 返回上级菜单
                                    }
                                    else
                                    {
                                        printf("无效的选择，请重新输入\n");
                                    }
                                }
                            }
                            else if (choice3 == 5)
                            {
                                printf("返回\n");
                                break; // 返回上级菜单
                            }
                            else if (choice3 == 6)
                            {
                                printf("退出\n");
                                return 0;
                            }
                            else
                            {
                                printf("无效的选择，请重新输入\n");
                            }
                        }
                    }
                }
                else if (choice2 == 2)
                {
                    printf(ANSI_COLOR_GREEN "(注册)请输入用户名:" ANSI_COLOR_RESET);
                    scanf("%s", sent_netdata.login.username);
                    printf(ANSI_COLOR_GREEN "(注册)请输入密码:" ANSI_COLOR_RESET);
                    scanf("%s", sent_netdata.login.password);
                    printf(ANSI_COLOR_GREEN "(注册)请再次输入密码:" ANSI_COLOR_RESET);
                    scanf("%s", buf);
                    // 比较两次输入的密码
                    if (strcmp(sent_netdata.login.password, buf) != 0)
                    {
                        printf("两次输入的密码不一致\n");
                        continue;
                    }
                    sent_netdata.command = CMD_REGIS;
                    nbytes = send(server_fd, &sent_netdata, sizeof(sent_netdata), 0);
                    if (-1 == nbytes)
                    {
                        perror("send error");
                        continue;
                    }
                    nbytes = recv(server_fd, &recv_netdata, sizeof(recv_netdata), 0);
                    if (-1 == nbytes)
                    {
                        perror("recv error");
                        continue;
                    }
                    if (0 == nbytes)
                    {
                        printf("服务器关闭\n");
                        break;
                    }
                    // 打印recv_netdata.result传过来的结果
                    printf("%s\n", recv_netdata.result);
                }
                else if (choice2 == 3)
                {
                    break; // 返回上级菜单
                }
                else
                {
                    printf("无效的选择，请重新输入\n");
                }
            }
        }
        else if (choice1 == 2)
        {
            printf("闲置选项\n");
        }
        else if (choice1 == 3)
        {
            printf("程序已退出\n");
            break; // 退出程序
        }
        else
        {
            printf("无效的选择，请重新输入\n");
        }
    }
    close(server_fd);
    return 0;
}

int send_recv_func(int server_fd, netdata_t *sent_netdata, netdata_t *recv_netdata)
{
    int ret;
    ret = send(server_fd, sent_netdata, sizeof(netdata_t), 0);
    if (-1 == ret)
    {
        perror("send error");
        return -1;
    }
    ret = recv(server_fd, recv_netdata, sizeof(netdata_t), 0);
    if (-1 == ret)
    {
        perror("recv error");
        return -1;
    }
    if (0 == ret)
    {
        printf("服务器关闭\n");
        return -1;
    }
    if (recv_netdata->result != NULL)
    {
        printf("%s\n", recv_netdata->result);
    }
    // 清空send_netdata、recv_netdata结构体
    // memset(sent_netdata, 0, sizeof(netdata_t));
}
