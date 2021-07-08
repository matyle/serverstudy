//单例模式  pthread_once线程安全的单例模式
#ifndef SingletonTON_H
#define SingletonTON_H
#include "../utils/noncopyable.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> // atexit
//懒汉模式 使用pthread_once线程安全的单例模式
//本函数使用初值为PTHREAD_ONCE_INIT的once_control变量保证init_routine()函数在本进程执行序列中仅执行一次。
template<typename T>//可以将单例模式写完模板类 表示给某个类取单例 例如log
class Singleton:Matt::noncopyable{
private:
    static T* obj_;
    static pthread_once_t ponce_;
    Singleton(){
    };
    ~Singleton(){
    };//私有化构造函数和析构函数

public:
    static T& getInstance(){
        //return (new Singleton);
        //static Singleton obj; //创建一个对象
        pthread_once(&ponce_,&Singleton::init); //函数名的地址
        return *obj_;//返回引用的对象 作为引用
    }

    static void init(){
        obj_ = new T();
        ::atexit(destory);//传函数名 即地址
    }

    static void destory(){
        delete obj_;
    }
};
template<typename T>
T* Singleton<T>::obj_ = NULL; //静态成员需要在内外定义！！
template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT; //静态成员需要在内外定义！！


#endif