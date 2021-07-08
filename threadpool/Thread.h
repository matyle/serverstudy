#ifndef THREAD_H
#define THREAD_H
#include <memory>
#include <pthread.h>
#include <functional>
#include "../utils/noncopyable.h"
#include "../utils/Atomic.h"
#include "Atomic.h"
//第四章多线程编程 来封装thread包

class Thread:Matt::noncopyable{

    
public:
    typedef std::function<void()> ThreadFunc; //
    Thread(const ThreadFunc&,const std::string& name = std::string()); //初始化线程执行的函数，以及线程名
    ~Thread();

    void start(); //创建一个线程
    void join();
    pid_t tid(){return tid_;}
    const std::string& name(){return name_;}


private:
    static void* startThread(void* thread);//static没有this指针，才能强转
    void runInthread();
    pid_t tid_;
    std::string name_;
    bool started_;
    ThreadFunc func_;//函数
    pthread_t pthreadId_;
    
    //static AtomicInt32 numCreated_;

};

namespace CurrentThread{
    pid_t tid();
    const char* name();
    bool isMainThread();//是否是主线程
}

#endif;