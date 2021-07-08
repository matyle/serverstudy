#ifndef CONDITION_H
#define CONDITION_H
#include "Mutex.h"
#include <boost/noncopyable.hpp>
class Condition : boost::noncopyable
{
public:
    explicit Condition(MutexLock &mutex) : mutex_(mutex)
    {
        pthread_cond_init(&pcond_, NULL);
    }
    //在c++种explicit关键字只能用来修饰构造函数。使用explicit可以禁止编译器自动调用拷贝初始化，
    //还可以禁止编译器对拷贝函数的参数进行隐式转换。

    ~Condition()
    {
        pthread_cond_destroy(&pcond_);
    }

    void wait()
    {
        pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
    }

    void notify()
    {
        pthread_cond_signal(&pcond_);
    }

    void notifyAll()
    {
        pthread_cond_broadcast(&pcond_);
    }

private:
    MutexLock &mutex_;
    pthread_cond_t pcond_;
};

//计数器
class CountDownLatch : boost::noncopyable
{
public:
    explicit CountDownLatch(int count) : mutex_(), condition_(mutex_), count_(count) {}

    void wait()
    {
        MutexLockGuard lock(mutex_);
        while (count_ > 0)
        {
            condition_.wait();
        }
    }
    void countDown()
    {

        //计数-1
        MutexLockGuard lock(mutex_);
        --count_;
        if (count_ == 0)
            condition_.notifyAll(); //为什么广播？
    }

private:
    mutable MutexLock mutex_; //顺序很重要，先mutex_后condition
    Condition condition_;
    int count_;
};
#endif