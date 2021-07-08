//利用阻塞队列，线程池，单例模式类实现一个日志类
#ifndef LOG_H
#define LOG_H
#include "../threadpool/ThreadPool.h"
#include "../utils/Mutex.h"
#include "threadpool/BoundBlocking_queue.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include <stdarg.h>
class Log{
private:
    static Log *log_;
    static pthread_once_t ponce_;
    Log(){}
    virtual ~Log(){}
    char dirName_[128]; //路径名
    char logName_[128];//log 文件名
    int splitLines_; //日志最大行数
    int logBufferSize_;  //日志缓冲区大小
    long long count_ ;//日志行数
    int today_ ;//日期
    FILE* fp_ ;//文件指针
    char* buf_ ;//要输出的内容
    
    

    Matt::BlockQueue<std::string> *blockQueue; //阻塞队列
    MutexLock mutex_;
    bool isAsync;//是否异步

public:
    static int closeLog_; //关闭日志
    static Log& getInstance(){
        pthread_once(&ponce_,&Log::instance);
        return *log_;
        ::atexit(destory);
    }
    static void instance(){
        log_ = new Log();
    }
    bool init(const char *file_name, int log_buf_size = 8192, 
                int split_lines = 5000000, int max_queue_size = 0);

    static void destory(){
        delete log_;
    }

    static void *flushLogfunc(void *args){ //必须是static的，不能有this指针
        Log::getInstance().asycnWrite();
    }

       //将输出内容按照标准格式整理
    void writeLog(int level, const char *format, ...);

    //强制刷新缓冲区
    void flush(void);
    

private:
    void *asycnWrite(){
        std::string singleLog;
        while(!blockQueue->empty()){
            MutexLockGuard lock(mutex_);
            fputs(singleLog.c_str(),fp_); //写入文件
        }   


    }
};
Log* Log::log_ = NULL;
pthread_once_t Log::ponce_ = PTHREAD_ONCE_INIT;

#define LOG_DEBUG(format, ...) if(0 == Log::closeLog_) {Log::getInstance().writeLog(0, format, ##__VA_ARGS__); Log::getInstance().flush();}
#define LOG_INFO(format, ...) if(0 == Log::closeLog_) {Log::getInstance().writeLog(1, format, ##__VA_ARGS__); Log::getInstance().flush();}
#define LOG_WARN(format, ...) if(0 == Log::closeLog_) {Log::getInstance().writeLog(2, format, ##__VA_ARGS__); Log::getInstance().flush();}
#define LOG_ERROR(format, ...) if(0 == Log::closeLog_) {Log::getInstance().writeLog(3, format, ##__VA_ARGS__); Log::getInstance().flush();}

#endif