#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include "../utils/Condition.h"
#include "../utils/Mutex.h"

#include "../utils/noncopyable.h"
#include <deque>
#include <assert.h>

namespace Matt{
template<typename T>
class BlockQueue{

private:
    MutexLock mutex_; //先定义锁
    Condition notEmpty;
    Condition notFull; //未满
    std::deque<T> queue_;
    size_t maxsize_;



public:
    BlockQueue(size_t maxsize=1024):mutex_(),notEmpty(mutex_),notFull(mutex_),
                            queue_(),maxsize_(maxsize){} //构造函数
    
    void post(const T& task,int status=-1{
        MutexLockGuard lock(mutex_);
        //检测有没有满
        while(queue_.full()){
            notFull.wait();//任务队列满 等待不满
        }
        assert(!queue_.full());
        if(status!=-1) 
        {
            task.m_check_state = status; // 耦合依赖性太强了。。。

        }
        queue_.push_back(task);
        notEmpty.notify();
    }
    bool full() const{
        MutexLockGuard lock(mutex_);
        if(queue_.size()>=maxsize_){
            return true;
        }
        return false;
    }

    bool empty() const{
        MutexLockGuard lock(mutex_);
        return queue_.empty();
    }
    T take(){
        MutexLockGuard lock(mutex_);
        while(queue_.empty()){ //必须用循环 
            notEmpty.wait(); //等待 任务队列为空 等待不空
        }
        assert(!queue_.empty());
        T task = queue_.front(); //取一个任务
        queue_.pop_front;
        notFull.notify(); //移除队列 通知队列不满
        return front;
    }

    size_t size(){
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

    size_t capactiy() const{
        MutexLockGuard lock(mutex_);
        return maxsize_;
    }
};

}

#endif 
