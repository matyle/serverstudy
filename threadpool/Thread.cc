#include "Thread.h"
#include <unistd.h>
#include <syscall.h>
#include <sys/types.h>

namespace CurrentThread
{
    __thread const char *t_threadName = "unkown";

}
__thread pid_t t_cacheTid = 0; //线程局部存储缓存tid



pid_t get_tid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
    //系统调用获取tid
}
//fork之后 返回之前调用
void afterFork()
{
    t_cacheTid = get_tid();
    CurrentThread::t_threadName = "main";
}

class ThreadNameInitializer
{
public:
    ThreadNameInitializer(){
        CurrentThread::t_threadName = "main";
        pthread_atfork(NULL, NULL, &afterFork); //创建之后会在返回之前调用afterFork
        //pthread_atfork()在fork()之前调用，当调用fork时，内部创建子进程前在父进程中会调用prepare，
        //内部创建子进程成功后，父进程会调用parent ，子进程会调用child。
    }
};

ThreadNameInitializer init; //初始化


Thread::Thread(const ThreadFunc& func,const std::string& name)
        :started_(false),
         pthreadId_(0),
         tid_(0),
         func_(func),
         name_(name)
{

} //初始化线程执行的函数，以及线程名


Thread::~Thread(){}