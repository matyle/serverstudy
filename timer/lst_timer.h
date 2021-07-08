#ifndef LST_TIEMR
#define LST_TIMER

#include <time.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 64
class UtilTimer;

struct client_data
{
    struct sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    UtilTimer *timer;
};

//定时器类 一个定时器是一个链表的结点
class UtilTimer
{
public:
    UtilTimer() : prev(NULL), next(NULL) {}

public:
    time_t expire;
    void (*cb_func)(client_data *);
    struct client_data *user_data;
    UtilTimer *prev; //指定前一个定时器
    UtilTimer *next; //指定下一个定时器
};

//升序链表
class SortTimerlst
{
private:
    UtilTimer *head;
    UtilTimer *tail;

public:
    SortTimerlst() : head(NULL), tail(NULL) {}
    ~SortTimerlst();
    void addTimer(UtilTimer *timer); //如果当前插入结点 小于头结点 直接插入
    void adjustTimer(UtilTimer *timer);
    void tick();
    void del(UtilTimer *timer);

private:
    void addTimer(UtilTimer *timer, UtilTimer *lst_head);
};

#endif