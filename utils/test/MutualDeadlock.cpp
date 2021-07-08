#include "../Mutex.h"
#include <pthread.h>
#include <set>
#include <stdio.h>
#include <memory>
#include <assert.h>
class Request;
typedef std::shared_ptr<Request> RequestPtr; //所有的用智能指针解决this指针暴露问题
class Inventory
{
public:
  //写端 copy
  void add(Request *req)
  {
    MutexLockGuard lock(mutex_);
    if(!requests_ptr.unique()){
    requests_ptr.reset(new Requestset(*requests_ptr));
    }
    assert(requests_ptr.unique());
    requests_ptr->insert(req);
  }

  void remove(Request *req) __attribute__((noinline))
  {
    MutexLockGuard lock(mutex_);
    if(!requests_ptr.unique()){
    requests_ptr.reset(new Requestset(*requests_ptr));
    }
    assert(requests_ptr.unique());
    requests_ptr->erase(req);
  }
  Inventory(){
    requests_ptr.reset(new Requestset);
  } //初始化

  void printAll() const; //const成员函数

private:
  mutable MutexLock mutex_;
  //std::set<Request *> requests_;
  typedef std::set<Request *> Requestset;
  typedef std::shared_ptr<Requestset> RequestsetPtr;
  mutable RequestsetPtr requests_ptr;
};

Inventory g_inventory;

class Request
{
public:
  void process() // __attribute__ ((noinline))
  {
    MutexLockGuard lock(mutex_);
    g_inventory.add(this);
    // ...
  }

  ~Request() __attribute__((noinline))
  {
    MutexLockGuard lock(mutex_);
    sleep(1);
    g_inventory.remove(this);
  }

  void print() const __attribute__((noinline))
  {
    MutexLockGuard lock(mutex_);
    // ...
  }

private:
  mutable MutexLock mutex_;
};
//读端
void Inventory::printAll() const //const成员函数
{
  // std::set<Request *> requests;
  // {
  
  //临界区
  RequestsetPtr requests_ptr_copy;
  {
  MutexLockGuard lock(mutex_);
  requests_ptr_copy = requests_ptr;
  }
  //   requests = requests_;
  // }
  //方法1 整个全部拷贝 太占内存。全部拷贝 锁只管requests_拷贝的时候
  //方法2 利用shared_ptr 引用计数
  
  sleep(1);
  for (std::set<Request *>::const_iterator it = requests_ptr_copy->begin();
       it != requests_ptr_copy->end();
       ++it)
  {
    (*it)->print();
  }
  printf("Inventory::printAll() unlocked\n");
}

/*
void Inventory::printAll() const
{
  std::set<Request*> requests
  {
    muduo::MutexLockGuard lock(mutex_);
    requests = requests_;
  }
  for (std::set<Request*>::const_iterator it = requests.begin();
      it != requests.end();
      ++it)
  {
    (*it)->print();
  }
}
*/

void *threadFunc(void* arg)
{
  Request *req = new Request;
  req->process();
  delete req;
  return NULL;
}

int main(void)
{
  pthread_t tid;
  pthread_create(&tid,NULL,threadFunc,NULL); //加了一个线程
  usleep(500 * 1000);
  g_inventory.printAll();
  pthread_join(tid, NULL);
  return 0;
}