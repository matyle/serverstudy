#include "../Mutex.h"
#include <pthread.h>
#include <stdio.h>
#define NITERS 10000000
void *count(void *arg);

unsigned int cnt = 0;// 全局变量
//sem_t mutex;
MutexLock mutex;

int main(){
    pthread_t tid[10];
    //sem_init(&mutex,0,1);
    // pthread_t tid1,tid2;
    // pthread_create(&tid1,NULL,count,NULL);
    // pthread_create(&tid2,NULL,count,NULL);
    // pthread_create(&tid3,NULL,count,NULL);
    // pthread_join(tid1,NULL);
    // pthread_join(tid2,NULL);

    for(int i=0;i<10;++i){
        pthread_create(&tid[i],NULL,count,NULL);
        
    }
    for(int i=0;i<10;++i){
        pthread_join(tid[i],NULL);
    }
    if(cnt != (unsigned)NITERS*10)
        printf("BOOM,cnt=%d\n",cnt);
    
    else{
        printf("OK,cnt=%d\n",cnt);
    }
    exit(0);
}

void* count(void *arg){
    int i;
     printf("myid:%d\n",pthread_self());
    for(i=0;i<NITERS;i++){
        MutexLockGuard lock(mutex);
        //sem_wait(&mutex);
        cnt++;
        //sem_post(&mutex);
       
    }
    
    return NULL;
}