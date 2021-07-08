void addFd(int fd,bool oneshot,int TRIGMode){
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

void modFd(int fd,int ev,int TRIGMode){
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

void removefd(int fd){
    epoll_ctl(epollfd_,EPOLL_CTL_DEL,fd,0);
    close(fd);
}
