#include <unistd.h>
#include <stdio.h>

#include "Timer.h"

Timer::Timer()
{
    this->isStart=false;  //初始化关闭线程标志
    this->wheel=new time_wheel();
    this->stack_index=0;
    this->timer_count=0;

    ///开启线程
    pthread_create(&this->pt,NULL,run,(void *)this);
    pthread_detach(this->pt); ///设置线程为分离态
}

void *Timer::run(void * arg)
{
    //静态转换
    Timer *t=static_cast<Timer *>(arg);

    while(t->isStart==false);
    while(t->isStart!=false)
    {
         sleep(1);
         //定时器运行
         t->wheel->tick();
         printf("tick\n");
    }
}

//运行函数体
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

    ///如果没有被废弃的timer_t
    if(this->stack_index<=0)
    {
        timers[this->timer_count++]=this->wheel->add_timer(timeout,handler);
        r=this->timer_count-1;

    }

    ///如果有被废弃的timer_t，则从stack中取出废弃的timer_t使用
    if(this->stack_index>0)
    {
        int index=stack[this->stack_index-1];
        timers[index]=this->wheel->add_timer(timeout,handler);
        r=index;
        this->stack_index--;

    }

    return r;

}


//删除定时器
int Timer::deleteTimer(w_timer_t timer_n)
{
    tw_timer *tw=timers[timer_n]; ///获取到timer

    if(tw!=NULL){
        this->wheel->del_timer(tw); //delete
        //delete tw;
        //tw已经在tw中delete了
        tw=NULL;
        stack[this->stack_index++]=timer_n; //将废弃的timer_t 存放到废弃堆上
        return 0;
    }

    return -1;
}


Timer::~Timer()
{
    delete [] this->timers;
    delete this->wheel;
}







