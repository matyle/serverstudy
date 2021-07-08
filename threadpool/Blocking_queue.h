
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
    Condition cond_;
    std::deque<T> queue_;

public:
    BlockQueue():mutex_(),cond_(mutex_),queue_(){} //构造函数  mutex_初始化条件变量的锁 是同一个锁
    
    void post(const T& task,stat){
        MutexLockGuard lock(mutex_);
        queue_.push_back(task);
        cond_.notify();
    }

    T take(){
        MutexLockGuard lock(mutex_);
        while(queue_.empty()){ //必须用循环
            cond_.wait(); //等待
        }
        assert(!queue_.empty());
        T task = queue_.front(); //取一个任务
        queue_.pop_front;
        return front;
    }

    int queue_size(){
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }
};

}

#endif 
