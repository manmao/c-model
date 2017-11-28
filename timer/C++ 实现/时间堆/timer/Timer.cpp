#include <unistd.h>
#include <stdio.h>
#include "Timer.h"

Timer::Timer(int count=10)
{
    this->isStart=false;  //初始化关闭线程标志
    this->heap=new time_heap(count);
    this->stack_index=0;
    this->timer_count=0;

    ///开启线程
    pthread_create(&this->pt,NULL,run,(void *)this);
    pthread_detach(this->pt); ///设置线程为分离态
    this->heap=new time_heap(10);
}

void *Timer::run(void * arg)
{
    Timer *t=static_cast<Timer *>(arg);

    while(t->isStart==false);
    while(t->isStart!=false)
    {
         sleep(1);
         //定时器运行
         t->heap->tick();
         printf("tick\n");
    }
}


void Timer::start()
{
    this->isStart=true;
}


void Timer::stop()
{
    this->isStart=false;
}


///添加定时器
w_timer_t Timer::addTimer(int timeout,TimerHandler *handler)
{
    w_timer_t r=-1;
    heap_timer *htimer=new heap_timer(timeout);
    htimer->handler=handler;

    ///如果没有被废弃的timer_t
    if(this->stack_index<=0)
    {
        this->heap->add_timer(htimer);
        timers[this->timer_count++]=htimer;
        r=this->timer_count-1;

    }

    ///如果有被废弃的timer_t，则从stack中取出废弃的timer_t使用
    if(this->stack_index>0)
    {
        int index=stack[this->stack_index-1];
        this->heap->add_timer(htimer);
        timers[index]=htimer;
        r=index;
        this->stack_index--;

    }

    return r;

}


//删除定时器
int Timer::deleteTimer(w_timer_t timer_n)
{
    heap_timer *ht=timers[timer_n]; ///获取到timer

    if(ht!=NULL){
        this->heap->del_timer(ht); //delete
        //delete tw;
        //tw已经在tw中delete了
        ht->handler=NULL;
        ht=NULL;
        stack[this->stack_index++]=timer_n; //将废弃的timer_t 存放到废弃堆上
        return 0;
    }

    return -1;
}


Timer::~Timer()
{
    delete [] this->timers;
    delete this->heap;

}







