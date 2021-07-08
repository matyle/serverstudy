//单例模式  pthread_once线程安全的单例模式
#include "../utils/noncopyable.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> // atexit
//懒汉模式 使用pthread_once线程安全的单例模式
//本函数使用初值为PTHREAD_ONCE_INIT的once_control变量保证init_routine()函数在本进程执行序列中仅执行一次。
//template<typename T>//可以将单例模式写完模板类 表示给某个类取单例
class Single:Matt::noncopyable{
private:
    static Single* obj_;
    static pthread_once_t ponce_;
    Single(){
        printf("structor\n");
    };
    ~Single(){
        printf("destructor\n");
    };//私有化构造函数和析构函数

public:
    static Single& getInstance(){
        //return (new Single);
        //static Single obj; //创建一个对象
        pthread_once(&ponce_,&Single::init); //函数名的地址
        return *obj_;//返回对象的地址
    }

    static void init(){
        obj_ = new Single();
        ::atexit(destory);//传函数名 即地址
    }

    static void destory(){
        delete obj_;
    }
    void print(){
        printf("hello world\n");
    }
};
Single* Single::obj_ = NULL; //需要在内外面声明！！
pthread_once_t Single::ponce_ = PTHREAD_ONCE_INIT;



//饿汉模式
//饿汉模式虽好，但其存在隐藏的问题，在于非静态对象（函数外的static对象）在不同编译单元中的初始化顺序是未定义的。
//如果在初始化完成之前调用 getInstance() 方法会返回一个未定义的实例。
// class Singleton{
// private:
//     static Singleton *obj;//Singleton的指针 静态的
//     Singleton(){printf("structor\n");}
//     ~Singleton(){}
// public:
//     static Singleton* getInstance();
//     void print(){
//         printf("hello world\n");
//     }

// };
// Singleton* Singleton::obj = new Singleton; //为什么写到外面可以？
// Singleton* Singleton::getInstance(){
//     return obj;
// }


int main(){
    //Single s;//错误
    Single &s = Single::getInstance();

    Single &s2 = Single::getInstance();
    //s s2实际上是同一个实例 只调用了一次构造函数，但是去析构两次，导致访问了空指针
    s.print();
    s2.print();
    // s->print();
    // s2->print();
    // s->destory();
    // s2->destory();
    /*
    structor 
    hello world
    hello world
    destructor
    munmap_chunk(): invalid pointer
    Aborted (core dumped)
    */

    // Singleton *sl = Singleton::getInstance();
    // Singleton *sl2 = Singleton::getInstance();//一个实例
    // sl->print();
    // sl2->print();

}