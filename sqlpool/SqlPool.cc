#include "SqlPool.h"
#include "../logger/Log.h"
using namespace std;

SqlPool::SqlPool(): empty_(mutex_),full_(mutex_){ //初始化条件变量
    this->curConn_ = 0;
    this->curFreeConn = 0;
}

void SqlPool::init(string url, string user, string passWord, string dataBaseName, int port, int maxConn, int closeLog){
    url_ = url;//网址
    user_ = user;
    passWord_ = passWord;
    databaseName_ = dataBaseName;
    port_ = port;
    maxConn_ = maxConn;
    closeLog_ = closeLog;
    
    for(int i=0;i<maxConn;++i){
        MYSQL *conn = NULL; 
        conn = mysql_init(conn);//初始化链接
        if(conn==NULL){
            LOG_ERROR("mysql error");
            exit(1);
        }
        conn = mysql_real_connect(conn,url_.c_str(),user_.c_str(),passWord_.c_str(),databaseName_.c_str(),
                                port_,NULL,0);

        if(conn==NULL){
            LOG_ERROR("mysql error");
            exit(1);
        }
        ++curFreeConn;
    }
    maxConn_ = curFreeConn;
    
}
MYSQL *SqlPool::getConnection(){
    MYSQL *conn = NULL; //局部指针变量 栈中存放的是地址
    MutexLockGuard lock(mutex_);
    while(conns_.empty()){
        empty_.wait();//空 等待
    }
    conn = conns_.front(); //存的是地址
    conns_.pop_front();

    --curFreeConn;
    ++curConn_;
    full_.notify();
    return conn; //返回地址（这个地址并不是指针变量的地址，而是指向conns元素的地址，因此可以 局部变量的指针和局部指针变量是两个不同概念
}

bool SqlPool::ReleaseConn(MYSQL *conn){
    if(conn == NULL) return false;
    MutexLockGuard lock(mutex_);
    while(conns_.size()>=maxConn_){
        full_.wait();
    }
    conns_.push_back(conn);
    ++curFreeConn;
    --curConn_;
    empty_.notify(); //条件变量
    return true;
}

void SqlPool::DestroyPool(){
    //会不会有竞争？ 只是清除队列吗？ 已经分配的怎么办？
    MutexLockGuard lock(mutex_);
    if(conns_.size()>0){
        auto it = conns_.begin();
        for(;it!=conns_.end();++it){
            MYSQL *conn = *it;
            mysql_close(conn);
        }
        curConn_ = 0;
        curFreeConn = 0;
        conns_.clear();
        delete sqlpool_;
    }
}
//当前空闲的连接数
int SqlPool::getFreeConn()
{
	return this->curFreeConn;
}
//为啥传**SQL 双重指针？
ConnectionRAII::ConnectionRAII(MYSQL *SQL, SqlPool *connPool){
	SQL = connPool->getConnection(); //返回指针
	
	conRAII = SQL;
	poolRAII = connPool;
}

ConnectionRAII::~ConnectionRAII(){
	poolRAII->ReleaseConn(conRAII);
}