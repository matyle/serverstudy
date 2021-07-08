#ifndef RIO_H
#define RIO_H
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include "Errorutils.h"
#define RIOBUF_SIZE 8192
//健壮性IO工具 大写代表健壮性工具，如果操作失败会报unix_error报错，免去了编写每次提示信息
ssize_t Read(int fd, void *buf, size_t count);
ssize_t Write(int fd, const void *buf, size_t count);
off_t Lseek(int fildes, off_t offset, int whence);
/*
* 健壮性IO rio相关
* rio_t
*/
typedef struct{
    int rio_fd;//内部缓冲区的描述符
    int rio_cnt; //内部缓冲区未读字节
    char *rio_bufp;//内部缓冲区下一个未读字节
    char rio_buf[RIOBUF_SIZE];//缓冲区
}rio_t;


ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd); 
ssize_t	rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t	rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

#endif