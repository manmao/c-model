#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "Timer.h"
#include "MyHandler.h"

int main(int argc,char *argv[])
{
	Timer *t=new Timer(10);

    MyHandler *handler=new MyHandler();
    client_data data;
    strcpy(data.buf,"alkdadsg\n");
    handler->setData(&data);
    t->start();

    w_timer_t t1=t->addTimer(3, handler);
    w_timer_t t2=t->addTimer(5, handler);
    w_timer_t t3=t->addTimer(9, handler);
    t->deleteTimer(t2);
    w_timer_t t4=t->addTimer(13, handler);
    w_timer_t t5=t->addTimer(11, handler);

    printf("%d %d %d %d",t1,t2,t3,t4,t5);



    while(1);

	return 0;
}