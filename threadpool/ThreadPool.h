#include <pthread.h>
#include "../utils/Mutex.h"
#include <boost/noncopyable.hpp>
#include "../utils/Condition.h"
#include "BoundBlocking_queue.h"
#include <exception>
#include <iostream>
//可以写成模板类，T必须要实现process函数
template <typename T>
class ThreadPool
{
public:
    ThreadPool(int thread_num, int max_tasks=10000);
    ~ThreadPool();
    bool addTask(T &task,int status=-1)const;

private:
    int thread_num_; //线程数量
    Matt::BlockQueue<T> workqueue_; //mutex，条件变量实现的任务队列
    //如果要条件变量只能定义在mutex_后面
    pthread_t *threads_;        //线程数组 可以用vector
    bool has_threads = false;
    bool runing_;

private:
    //私有静态成员函数
    static void *worker(void *arg); //每个对象只有一个副本 
    void run()                      //run函数
};


