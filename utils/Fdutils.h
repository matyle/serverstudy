#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <error.h>
#define LISTENQ 1024 /* second argument to listen() */
class FdUtils{
public:
    static int *pipefd_;
    static void sig_handler(int sig);
    

};
int openlistenfd(int port)
{
    //struct hostent *hp;
    int ret;
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //

    //inet_pton(AF_INET,INADDR_ANY,&server_addr.sin_addr.s_addr);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        printf("listen failed\n");
        return -1;
    }
    //绑定
    ret = bind(listenfd, (struct sockaddr *)(&server_addr), sizeof(server_addr));
    if (ret == -1)
    {
        printf("bind failed\n");
        exit(0);
    }

    ret = listen(listenfd, LISTENQ); //绑定之后开始监听
    if (ret == -1)
    {
        printf("bind failed\n");
        exit(0);
    }
    printf("listen...\n");
    return listenfd;
}

int openclientfd(char *hostname, int port)
{
    struct hostent *hp;
    int ret, sockfd;
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if ((hp = gethostbyname(hostname)) == NULL)
    {
        printf("hostname error\n");
        return -2;
    }
    bcopy((char *)hp->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, sizeof(hp->h_length));

    //inet_pton(AF_INET,hostname,&server_addr.sin_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(server_addr)) == -1)
    {
        printf("connect error\n");
        return -1;
    }
    return sockfd;
}

int setnonbloking(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt); //设置 非阻塞
    return old_opt;              // 返回旧的选项
}
void FdUtils::sig_handler(int sig)
{ //信号处理函数
    int save_errno = errno;
    int msg = sig;
    send(pipefd_[1], (char *)&msg, 1, 0); //将信号值写入管道，通知主循环 pipefd[0]读出，pipefd[1]写入管道
    errno = save_errno;
}
//设置信号函数
void addsig(int sig, void(handler)(int), bool restart = true)
{
    //创建sigaction结构体变量
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));

    //信号处理函数中仅仅发送信号值，不做对应逻辑处理
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    //将所有信号添加到信号集中
    sigfillset(&sa.sa_mask);
    //执行sigaction函数
    assert(sigaction(sig, &sa, NULL) != -1);
}
