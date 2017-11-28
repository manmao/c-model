/*************************
 将TimerHandler注册到运行级别
*************************/

#ifndef __TIMERHANDLER_H_
#define __TIMERHANDLER_H_

#include <stdlib.h>

class TimerHandler{
public:
     TimerHandler();
     ///设置定时器到来时，运行函数需要的数据
     void setData(void *arg);
     ///获取数据
     void* getData();
     virtual ~TimerHandler();

     //定时器超时处理函数，需要在
     virtual void handle(void *arg)=0;

private:
    void *data;
};









#endif
