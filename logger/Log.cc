#include "Log.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
bool Log::init(const char *file_name, int log_buf_size = 8192,
               int split_lines = 5000000, int max_queue_size = 0)
{
    blockQueue = new Matt::BlockQueue<std::string>(max_queue_size);
    pthread_t tid;

    //flush_log_thread为回调函数,这里表示创建线程异步写日志
    pthread_create(&tid, NULL, flushLogfunc, NULL);

    //缓冲区大小
    logBufferSize_ = log_buf_size;
    buf_ = new char[logBufferSize_];
    memset(buf_, '\0', sizeof(buf_));

    splitLines_ = split_lines;

    time_t t = time(NULL); //时间
    struct tm *sysTm = localtime(&t);
    struct tm myTm = *sysTm;

    const char *p = strrchr(file_name, '/');
    char logFullName[256] = {0};

    if (p == NULL)
    {
        snprintf(logFullName, 255, "%d_%02d_%02d_%s", myTm.tm_year + 1900, myTm.tm_mon + 1, myTm.tm_mday, file_name);
    }
    else
    {
        strcpy(logName_, p + 1);
        strncpy(dirName_, file_name, p - file_name + 1);
        snprintf(logFullName, 255, "%s%d_%02d_%02d_%s", dirName_, myTm.tm_year + 1900, myTm.tm_mon + 1, myTm.tm_mday, logName_);
    }

    today_ = myTm.tm_mday;

    fp_ = fopen(logFullName, "a");
    if (fp_ == NULL)
    {
        return false;
    }

    return true;
}

void Log::writeLog(int level, const char *format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm *sysTm = localtime(&t);
    struct tm my_tm = *sysTm;
    char s[16] = {0};
    switch (level)
    {
    case 0:
        strcpy(s, "[debug]:");
        break;
    case 1:
        strcpy(s, "[info]:");
        break;
    case 2:
        strcpy(s, "[warn]:");
        break;
    case 3:
        strcpy(s, "[erro]:");
        break;
    default:
        strcpy(s, "[info]:");
        break;
    }
    //写入一个log，对m_count++, m_split_lines最大行数
    { //临界区1
    MutexLockGuard lock(mutex_);
    count_++;

    if (today_ != my_tm.tm_mday || count_ % splitLines_ == 0) //everyday log
    {

        char new_log[256] = {0};
        fflush(fp_);
        fclose(fp_);
        char tail[16] = {0};

        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

        if (today_ != my_tm.tm_mday)
        {
            snprintf(new_log, 255, "%s%s%s", dirName_, tail, logName_);
            today_ = my_tm.tm_mday;
            count_ = 0;
        }
        else
        {
            snprintf(new_log, 255, "%s%s%s.%lld", dirName_, tail, logName_, count_ / splitLines_);
        }
        fp_ = fopen(new_log, "a");
    }

    }

    va_list valst;
    va_start(valst, format);

    std::string logStr;
    {
    MutexLockGuard lock(mutex_);

    //写入的具体时间内容格式
    int n = snprintf(buf_, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);

    int m = vsnprintf(buf_ + n, logBufferSize_ - 1, format, valst);
    buf_[n + m] = '\n';
    buf_[n + m + 1] = '\0';
    logStr = buf_;
    }

    if (isAsync && !blockQueue->full())
    {
        blockQueue->post(logStr);
    }
    else
    {
        MutexLockGuard lock(mutex_);
        fputs(logStr.c_str(), fp_);
    }

    va_end(valst);
}

void Log::flush(void)
{
    MutexLockGuard lock(mutex_);
    //强制刷新写入流缓冲区
    fflush(fp_);
}