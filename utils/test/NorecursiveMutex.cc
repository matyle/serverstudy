#include "../Mutex.h"
#include <vector>
class Foo{
public:
    void doit() const;
};




MutexLock mutex;
std::vector<Foo> foos;

void postWithlockhold(const Foo& f){
    foos.push_back(f);// 
}

void post(const Foo& f){
    MutexLockGuard lock(mutex);
    postWithlockhold(f);
}


void traverse(){
    MutexLockGuard lock(mutex);
    for(auto beg = foos.begin();beg!=foos.end();++beg){
        beg->doit(); //调用post 死锁 //如果是不加锁版本 损坏数据
    }
}

void Foo::doit() const{
    Foo f;
    post(f); //
}

int main(){
    Foo fo;
    post(fo);
    traverse();
}