#include "TimerHandler.h"

TimerHandler::TimerHandler(){
     this->data=NULL;
}

///设置定时器到来时，运行函数需要的数据
void TimerHandler::setData(void *arg){
    this->data=arg;
}

///获取数据
void* TimerHandler::getData(){
     return this->data;
}

TimerHandler::~TimerHandler(){

}

