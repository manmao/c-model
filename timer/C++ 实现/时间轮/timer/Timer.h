/*****************************************
    封装对定时器的操作


********************************************/
#ifndef __TIMER_H_
#define __TIMER_H_

#include <pthread.h>
#include <stdlib.h>


#include "TimerHandler.h"
#include "TimeWheel.h"


// #define TIMER_MAX_SIZE 1000

typedef int w_timer_t;

class Timer{
public:
    Timer();
    virtual ~Timer();
    //定时器开启
    virtual void start();
    //定时器关闭
    virtual void stop();
    //添加定时器
    virtual w_timer_t addTimer(int timeout,TimerHandler *handler);
    //关闭定时器
    virtual int deleteTimer(w_timer_t timer_n);
private:
    ///C++定义函数指针的用法，用于线程运行体
    static void *run(void *arg);
private:
    time_wheel *wheel;      //时间轮对象，负责底层的操作
    pthread_t pt;           //定时器运行线程
    tw_timer *timers[65536]; //保存当前申请的定时器
    int timer_count;        ///记录当前定时器下标

private:
    bool isStart;      //开启定时器的标志
    int  stack[65536]; ///存放被删除的定时器的坐标
    int  stack_index; ///记录删除的定时器的下标

};













#endif
