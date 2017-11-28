#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "Timer.h"
#include "TimeWheel.h"
#include "TimerHandler.h"
#include "MyHandler.h"


int main(int argc,char *argv[])
{
     Timer *timer=new Timer(); //新建一个定时器

    //创建一个Handler
     MyHandler *handler=new MyHandler();

     struct client_data data;//=(client_data *)calloc(1,sizeof(struct client_data));

     strcpy(data.buf,"Ceshi xiaoxi");

     handler->setData(&data);

     timer->start(); //打开定时器

     //添加定时器
     w_timer_t t1=timer->addTimer(3,handler);

     w_timer_t t2=timer->addTimer(5,handler);

     //删除定时器
     timer->deleteTimer(t1);
     w_timer_t t3= timer->addTimer(10,handler);

     w_timer_t t4= timer->addTimer(20,handler);

     timer->deleteTimer(t4);

     w_timer_t t5=timer->addTimer(8,handler);

     printf("%d %d %d %d %d",t1,t2,t3,t4,t5);

     while(1);

    return 0;
}