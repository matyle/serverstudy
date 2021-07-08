#include<iostream>
#include "openfd.h"
#include<pthread.h>
#include<stdio.h>
#include<unistd.h>
#define BUFFERSIZE 1024
#define NTHREADS 10
void echo(int connfd);
void* threadfunc(void* args);
int main(int argc,char** argv){
    int *connfdp,optval=1;
    pthread_t tid;
    struct sockaddr_in clientaddr;
    
    int clientaddr_len = sizeof(clientaddr);

    bzero(&clientaddr,clientaddr_len);
    if(argc<=1){
        std::cout<<"input port"<<std::endl;
        return 1;
    }
    int port = atoi(argv[1]);
    int listenfd = openlistenfd(port);
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, 
		   (const void *)&optval , sizeof(int)) < 0)
	    return -1;
    printf("listened\n");
    //链接
    while (1)
    {
        connfdp = (int*)malloc(sizeof(int));
        *connfdp = accept(listenfd,(struct sockaddr*)(&clientaddr),(socklen_t*)(&clientaddr_len));
        pthread_create(&tid,NULL,threadfunc,connfdp);
    }
    pthread_exit(NULL);
    
}

void* threadfunc(void* args){
    //pthread_detach(pthread_self());
    int connfd = *((int *)args);//先转换成int* 然后引用
    free(args);
    echo(connfd);
    close(connfd);
    return NULL;
}

void echo(int connfd){
    char buf[BUFFERSIZE];
    memset(buf,'\0',BUFFERSIZE);
    int n;
    while (1)
    {
        if((n=read(connfd,buf,BUFFERSIZE))>0){
            std::cout<<"have recv " << n << " bytes"<<std::endl;
            write(connfd,buf,BUFFERSIZE);
        };
    }
    close(connfd);
}