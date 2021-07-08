#ifndef SQLPOOL_H
#define SQLPOOL_H
#include "../utils/Mutex.h"
#include<mysql/mysql.h>
#include<string>
#include<deque>
#include <stdlib.h> // atexit
#include "../threadpool/BoundBlocking_queue.h"
#include "../utils/Condition.h"
#include <pthread.h>
using namespace std;
//连接池的功能主要有：初始化，获取连接、释放连接，销毁连接池。
class SqlPool{
public:
	string url_;			 //主机地址
	int port_;		 //数据库端口号
	string user_;		 //登陆数据库用户名
	string passWord_;	 //登陆数据库密码
	string databaseName_; //使用数据库名
	//int closeLog_;	//日志开关
    static SqlPool* getInstance()
    {
        pthread_once(&conce_,SqlPool::init_sqlpool);
        return sqlpool_;
    }
    static void init_sqlpool()//初始化对象指针
    {   
        sqlpool_ = new SqlPool();
        //::atexit(dele);
    } 
    bool ReleaseConn(MYSQL *conn);//释放连接
    MYSQL *getConnection();
    void init(string url, 
              string user, 
              string passWord, 
              string dataBaseName, 
              int port, 
              int maxConn, 
              int close_log); 


private:
    SqlPool();
    ~SqlPool(){};
    static void dele(){
        delete sqlpool_;
    }
    void DestroyPool();
    int getFreeConn();

    static SqlPool *sqlpool_;
    static pthread_once_t conce_;
   
    
    MutexLock mutex_;
    Condition empty_;
    Condition full_;
    int maxConn_;//最大连接数
    int curConn_;//当前已使用的连接
    int curFreeConn; //最大空闲的连接数
    std::deque<MYSQL*> conns_;// 连接数


};

//初始化
pthread_once_t SqlPool::conce_ = PTHREAD_ONCE_INIT;
SqlPool *SqlPool::sqlpool_ = NULL;

class ConnectionRAII{

public:
	ConnectionRAII(MYSQL *con, SqlPool *connPool);
	~ConnectionRAII();
	
private:
	MYSQL *conRAII;
	SqlPool *poolRAII;
};


#endif