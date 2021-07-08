#ifndef MUTEX_H
#define MUTEX_H
#include <pthread.h>
#include "./noncopyable.h"
class MutexLock : Matt::noncopyable
{
    //禁止拷贝，赋值运算符
    //构造函数
public:
    MutexLock()
    {
        pthread_mutex_init(&mutex_, NULL);
    }
    ~MutexLock()
    {
        holder_ = 0;
        pthread_mutex_destroy(&mutex_);
    }
    // bool isLockedByThisThread(){
    //     return holder_ == CurrentThread::tid();
    // }

    //void assertLocked();

    void lock()
    {
        pthread_mutex_lock(&mutex_);
        //holder_ = CureentThread::tid();
    }

    void unlock()
    {
        holder_ = 0;
        pthread_mutex_unlock(&mutex_);
        //
    }
public:
    pthread_mutex_t *getPthreadMutex()
    { //严禁用户代码调用，条件变量使用
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;
    pid_t holder_;
};

//栈上对象MutexLockGuard的生命期就是临界区长度。省去了自己加锁解锁
class MutexLockGuard : Matt::noncopyable
{
public:
    explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex)
    {
        mutex_.lock();
    }
    ~MutexLockGuard()
    {
        mutex_.unlock();
    }

private:
    MutexLock &mutex_;
};
#define MutexLockGuard(x) error "Missing guard object name"
//防止这么写 MutexLockGuard(mutex) 错误
#endif
