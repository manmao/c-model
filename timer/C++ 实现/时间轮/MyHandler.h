#ifndef  __MYHANDLER_H_
#define  __MYHANDLER_H_

#include "TimerHandler.h"

/*绑定socket和定时器*/
struct client_data
{
	struct sockaddr_in address;
	int sockfd;
	char buf[BUFFER_SIZE];
	tw_timer* timer;

};

class MyHandler : public TimerHandler{

public:
    MyHandler(){

    }

    virtual void handle(void *arg){
        client_data  *data1=(client_data *)arg;
        printf("%s\n",data1->buf);

        printf("timeout\n");
    }
};



#endif
