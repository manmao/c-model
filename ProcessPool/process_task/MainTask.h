#ifndef __MAIN_TASK_H_
#define __MAIN_TASK_H_

#include "SipServer.h"

class MainTask{
public:
	MainTask();
	~MainTask();
public:
	//
	int init(int argc,char *argv[]);
	
	//
	void run();

private:
	
};


#endif
