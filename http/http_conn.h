#ifndef HTTPCONNH
#define HTTPCONNH
#include <sys/types.h> //重命名
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include<sys/mman.h> //共享内存
#include<stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include<pthread.h>
#include"../utils/Mutex.h"
#include <sys/uio.h>
#include "../utils/Fdutils.h" //sock相关
#include <sys/uio.h>
#include "../sqlpool/SqlPool.h"
#include "../logger/Log.h"
#include <memory>
#include<map>

//#include "../epoll/Epoll.h"
// #define READ_BUFFER_SIZE 1024
// #define WRITE_BUFFER_SIZE 1024
/*
浏览器端发出http连接请求，主线程创建http对象接收请求并将所有数据读入对应buffer，
将该对象插入任务队列，工作线程从任务队列中取出一个任务进行处理

工作线程取出任务后，调用process_read函数，通过主、从状态机对请求报文进行解析。

解析完之后，跳转do_request函数生成响应报文，通过process_write写入buffer，返回给浏览器端
*/
class Epoll;
class http_conn{

public:
    //定义大小
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048; //读缓冲大小
    static const int WRITE_BUFFER_SIZE = 1024; //写缓冲大小
    //http请求方法，我们只支持GET 和POST
    enum METHOD{GET=0,POST,HEAD,PUT,DELETE,TRACE,OPTIONS,CONNECT,PATCH};

    //解析客户请求时 主状态机的状态
    enum CHECK_STATE{CHECK_STATE_REQUESTLINE = 0,
                    CHECK_STATE_HEADER,
                    CHECK_STATE_CONTENT};
    //NO_REQUEST请求不完整，需要继续读取请求报文数据
    //GET_REQUEST获得了完整的HTTP请求
    enum HTTP_CODE{NO_REQUEST,GET_REQUEST,BAD_REQUEST,NO_RESOURE,FROBIDDEN_REQUEST,FILE_REQUEST,INTERNAL_ERROR,CLOSED_CONNECTION};

    enum LINE_STATUS{LINE_OK=0,LINE_BAD,LINE_OPEN}; //LINE_OPEN读取的行不完整 LINE_BAD，报文语法有误

//私有数据成员
private:
    int m_sockfd;//fd
    struct sockaddr_in m_address; //对方socket地址
    MutexLock mutex_;

    //读缓冲区
    char m_read_buf[READ_BUFFER_SIZE];
    //标记读缓冲已读入客户数据的最后一个字节的下一个位置 读入buf
    int m_read_idx;     
    //正在分析的字符在读缓冲区中的位置
    int m_checked_idx;
    //当前正在解析行的起始位置
    int m_start_line;
    //写缓冲区
    char m_write_buf[WRITE_BUFFER_SIZE];

    int m_write_idx;//写缓冲区待发送的字节数

    CHECK_STATE m_check_state;

    METHOD m_method;
    int cgi;//初始化为0

    char m_real_file[FILENAME_LEN];//目标文件的完整路径，doc_root+m_url doc_root是网站根目录
    char *m_url;//请求目标的文件名
    char *m_version;
    char *m_host;
    int m_content_length;
    bool m_linger;//是否保持链接
    char* m_file_address;//请求的目标文件被mmap到内存中的起始位置

    char* m_string;

    //目标文件的状态信息，元文件信息 文件结构信息
    struct stat m_file_stat;
    struct iovec m_iv[2];//writev执行写操作，定义两个成员
    int m_iv_count; //写入内存块的数量

    MYSQL *mysql_;


    char *doc_root;

    map<string, string> m_users;
    int m_TRIGMode;
    int m_close_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];

    //SqlPool sqlpool_;
    //Epoll epoll_;
//公有静态成员
public:
    static shared_ptr<Epoll> epollPtr_;//静态成员，类中所有对象的socket事件都存在一个epoll内核注册表中 如何得到？
    static int m_user_count;//统计用户数量

public:
    http_conn(){}
    ~http_conn(){}

public:
    void init(int sockfd,const sockaddr_in& addr,char* root,int TRIGMode,int close_log,string user, string passwd, string sqlname);//初始化新接受的链接
    void initmysql(SqlPool *sqlpool);
    void close_conn(bool real_close = true);
    void process();//处理客户请求
    bool read_once();//非阻塞读 只读一次 读完立即返回 直到无数据可读或对方关闭连接 ，读完之后更新m_read_idx。即指向已读的最后一个字节的下一个位置，等待继续读
    bool write();//非阻塞写 响应报文写入


    int timer_flag;
    int improv;

private:
    void init();//初始化链接
    HTTP_CODE process_read(); //解析http请求
    bool process_write(HTTP_CODE ret);//填充http应答

    //下面的一组函数被process_read调用以分析HTTP请求
    HTTP_CODE parse_requset_line(char* text);//解析行
    HTTP_CODE parse_headers(char* text);
    HTTP_CODE parse_content(char* text);

    HTTP_CODE do_request();
    //指向一行的开始，\n结束
    char* get_line(){return m_read_buf+m_start_line;}

    LINE_STATUS parse_line();

    //下面这一组函数process_write调用以 填充HTTP应答 do_request调用
    void unmap();
    bool add_response(const char* format,...);
    bool add_content(const char* content);
    bool add_status_line(int status,const char* title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();
};

#endif