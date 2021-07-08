//Copy on write
/*
post main函数post 这时 unique为true
post
reset...
copy the whole list
*/
#include "../Mutex.h"
#include <vector>
#include <memory>
#include <stdio.h>
#include <assert.h>
#include <boost/shared_ptr.hpp>
class Foo
{
public:
    void doit() const;
};

MutexLock mutex;
typedef std::vector<Foo> FooList;
typedef boost::shared_ptr<FooList> FooListPtr; 
FooListPtr g_foos; //容器的智能指针 需要初始化！！！！


void post(const Foo &f)
{
    printf("post\n"); 
    //临界区
    {
    //printf("get_count:%ld\n",g_foos.use_count());
    MutexLockGuard lock(mutex);
    if (!g_foos.unique()) //如果有人引用
    {
        //复制一份 引用计数清为11
        g_foos.reset(new FooList(*g_foos));//为什么初始化为FooList(g_foos*)出错 没有初始化
        
    }
    if(!g_foos.unique()){ //被引用了
        exit(1);
    }
    g_foos->push_back(f);

    }
    printf("copy the whole list\n");
    // if (g_foos.unique())
    // {
    //     exit(0);
    // }
    //assert(g_foos.unique());
    
}

void traverse()
{
    FooListPtr foos,foos2;
    //临界区
    {
        MutexLockGuard lock(mutex);
        foos = g_foos; //引用计数+1 unique 为false
        foos2 = g_foos;
        if (g_foos.unique())
        {
            exit(1);
        }
        //assert(!g_foos.unique()); //断言g_foos是unique()就退出
    }
    //assert(!foos.unique());
    for (std::vector<Foo>::const_iterator it = foos->begin(); it != foos->end(); ++it)
    {
        it->doit();
    }
}

void Foo::doit() const
{
    Foo f;
    post(f); //
}

int main()
{
    g_foos.reset(new FooList);//初始化gfoos 初始化容器指针
    Foo fo;
    post(fo);
    printf("get_count:%ld\n",g_foos.use_count());
    traverse();
    //traverse();
    // Foo fo1;
    // post(fo1);
    // printf("get_count:%ld\n",g_foos.use_count());

}

