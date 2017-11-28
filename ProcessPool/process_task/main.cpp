#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

#include <iostream>
#include "Process.h"
#include "MainTask.h"

int main(int argc,char *argv[])
{

	Process<MainTask> *p=new Process<MainTask>();

	//new task
	MainTask *t=new MainTask();

	//start run process
	p->init(t,argc,argv);
	p->start();

	delete p;
	delete t;

	return 0;
}
