#include "lst_timer.h"
SortTimerlst::~SortTimerlst()
{
    UtilTimer *tmp = head;
    while (tmp)
    {
        //
        head = tmp->next;
        delete tmp; //实际上删了第一个
        tmp = head; //直到head为空
    }
}
void SortTimerlst::addTimer(UtilTimer *timer)
{ //添加timer到链表中
    if (!timer)
    {
        return;
    }
    if (!head)
    {
        head = tail = timer;
        return;
    }
    //timer的时间小于头结点直接插入
    if (timer->expire < head->expire)
    {
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }
    addTimer(timer, head);
}
void SortTimerlst::adjustTimer(UtilTimer *timer)
{
    //某个定时任务发生变化时，调整对应的定时器在链表中的位置
    if (!timer)
    {
        return;
    }
    UtilTimer *tmp = timer->next;
    if (!tmp || (timer->expire < tmp->expire))
    {
        return;
    }
    if (timer == head)
    {
        head = head->next;
        head->prev = NULL;
        timer->next = NULL;
        addTimer(timer, head);
    }
    else
    {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        addTimer(timer, timer->next);
    }
}
void SortTimerlst::del(UtilTimer *timer)
{
    if (!timer)
        return;
    if (timer == head && timer == tail)
    {
        //只有一个定时器
        delete timer;
        head = NULL;
        tail = NULL;
        return;
    }
    if (timer == head)
    {

        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    if (timer == tail)
    {
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}
void SortTimerlst::tick()
{ //执行信号处理函数
    if (!head)
    {
        return;
    }
    std::cout << "timer tick\n"
              << std::endl;
    time_t cur = time(NULL); //当前时间
    UtilTimer *tmp = head;
    //从头结点依次处理每个定时器
    while (tmp)
    {
        if (cur < tmp->expire)
        {
            //未超时
            break;
        }
        tmp->cb_func(tmp->user_data);
        head = tmp->next;
        if (head)
            head->prev = NULL;
        delete tmp;
        tmp = head;
    }
}
//需要遍历链表找到第一个超时时间大于timer的超时时间在他之前插入
void SortTimerlst::addTimer(UtilTimer *timer, UtilTimer *lst_head)
{
    UtilTimer *prev = lst_head;
    UtilTimer *tmp = prev->next;
    //遍历链表找到一个超时时间大于timer的超时时间在他之前插入
    while (tmp)
    {
        if (timer->expire < tmp->expire) //timer
        {
            //插在tmp之前
            timer->next = tmp;
            timer->prev = tmp->prev;
            tmp->prev->next = timer;
            tmp->prev = timer;
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    //没找到插到尾结点
    if (!tmp)
    {
        //没找到 成为新的尾结点
        prev->next = timer;
        timer->prev = prev;
        timer->next = NULL;
        tail = timer;
    }
}