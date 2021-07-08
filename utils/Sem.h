#include<semaphore.h>
#include "MutexLock.h"
class Sem
{
private:
    sem_t sem;
    //MutexLock mutex_;
public:
    Sem(/* args */); //默认为信号量的值1
    Sem(int value);//信号量的值

    ~Sem(); //销毁信号量
    bool P(sem_t& sem); //wait操作
    bool V(sem_t& sem); //sem_post操作
};

Sem::Sem(/* args */)
{
    sem_init(&sem,0,1);
}

Sem::Sem(int value)
{
    sem_init(&sem,0,value); //初始化
}


Sem::~Sem()
{
    sem_destroy(&sem);
}

bool Sem::P(sem_t &sem){
    return sem_wait(&sem)==0;
}

bool Sem::V(sem_t &sem){
    return sem_post(&sem)==0;
}