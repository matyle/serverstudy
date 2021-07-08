#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>
#include "../utils/Errorutils.h"
#include <errno.h>
class Epoll
{
public:
    Epoll(int maxEvent=1024)
    {
        this->epollfd_ = epoll_create(5); //
        if(this->epollfd_==-1){
            unix_error("Epoll creat failed");
        }
    }
    ~Epoll(){
    }
    void Add_fd(int fd,bool oneshot,int TRIGMode);
    void Mod_fd(int fd,int ev,int TRIGMode);
    void Remove_fd(int fd);
    void Epoll_wait();

private:
    int event_num_;       // epoll事件数
    epoll_event *events_; //epoll事件
    int epollfd_;
    int nready_;
};
#endif