#include "Epoll.h"
#include "../utils/Fdutils.h"
void Epoll::Add_fd(int fd,bool oneshot,int TRIGMode){
    struct epoll_event event;
    event.data.fd = fd;
    if(TRIGMode==1){
        event.events = EPOLLIN|EPOLLET|EPOLLRDHUP;
    }
    else{
        event.events = EPOLLIN|EPOLLRDHUP;
    }
    if(oneshot){
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd_,0,EPOLL_CTL_ADD,&event);
    setnonbloking(fd);
}

void Epoll::Mod_fd(int fd,int ev,int TRIGMode){
    epoll_event event;
    event.data.fd = fd;
    if(TRIGMode==1){
        event.events = ev|EPOLLET|EPOLLRDHUP|EPOLLONESHOT;
    }
    else{
        event.events = ev|EPOLLONESHOT|EPOLLRDHUP;
    }
    epoll_ctl(epollfd_,EPOLL_CTL_MOD,fd,&event);
}

void Epoll::Remove_fd(int fd){
    epoll_ctl(epollfd_,EPOLL_CTL_DEL,fd,0);
    close(fd);
}

void Epoll::Epoll_wait(){
    int nready;
    do{
        nready= epoll_wait(epollfd_,events_,event_num_,0);
    }while(nready==-1&&errno==EINTR);
    if(nready == -1){
        unix_error("Epoll wait failed");
    }
    nready_ = nready;

}




