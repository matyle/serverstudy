#include "ThreadPool.h"
template <typename T>
ThreadPool<T>::ThreadPool(int thread_num, int max_tasks) : thread_num_(thread_num),
                                                           workqueue_(max_tasks), runing_(true)
{
    if (thread_num_ < 0 || max_requests<0)
    {
        throw std::exception();
    }
    //创建动态数组
    threads_ = new pthread_t[thread_num_];
    if (!threads_)
        throw std::exception();

    //创建线程数组（池）两个失败的点，异常前一定要先释放内存
    for (int i = 0; i < thread_num_; ++i)
    {
        if (pthread_create(threads_[i], NULL, worker, this) != 0)
        {
            //创建线程失败，删除动态分配的内存、
            delete[] thresads_;
            throw std::exception();
        }
        //分离线程 不互相等待结束
        //如果用pthread_join(pthread_id)后，如果该线程没有运行结束，调用者会被阻塞，想想badcnt
        if (pthread_detach(threads_[i]) != 0)
        {
            //分离失败 删除已分配内存
            delete[] threads_;
            throw std::exception();
        }
    }
    has_threads = true;
}
template <typename T>
ThreadPool<T>::~ThreadPool()
{
    delete[] threads_;
    runing_ = false;
}

template <typename T>
bool ThreadPool<T>::addTask(T &task,int state=-1)const{
    
    workqueue_.post(task，state); //封装好的blocking queue
    return true;
}

template <typename T>
void *ThreadPool<T>::worker(void *args)
{
    ThreadPool *pool = (ThreadPool *)(args);
    pool->run();
    return pool;
}

template <typename T>
void ThreadPool<T>::run()
{
    //取出任务运行 等待是否唤醒
    while (runing_)
    {
        T task = workqueue_.take();
        //取出一个数据库连接
        
        task->proccess();
        //数据库
    }
}