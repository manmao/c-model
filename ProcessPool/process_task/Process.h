#ifndef __PROCESS_H_
#define __PROCESS_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

/*双进程模型为：
	Process.init()==>
	 1.首先初始化MainTask类的init()函数
	 2.启动(父进程)和(子进程)
	
	Process.child_process()===>
	  3.子进程启动执行MainTask类的run()函数 
     
	Process.parent_process()==>
	  4.父进程监听子进程退出情况     
	    当子进程异常退出，父进程将再次执行初始化，跳到第1步执行*/
//
//T must implment init() and run()
//


template <typename T>
class Process{
public:
	Process();	
	virtual ~Process();
public:
	void init(T *t,int argc,char *argv[]);
	void start();

private:
	 void child_process();
	 int parent_process(int argc,char *argv[]);

private:
	T *task;
	pid_t process_id;

};

//

template<typename T>
Process<T>::Process(){
	process_id=-1;
}


template<typename T>
void Process<T>::init(T *t,int argc,char *argv[])
{
	 this->task=t;
	 //初始化任务类
	 this->task->init(argc,argv);    ///T must have method [void init()]

	 process_id=fork();
	 if(process_id<0)
	 {
	 	perror("fork");
	 	printf("Process error FIle: %s,Line :%d",__FILE__,__LINE__);
	 	return ;
	 }

	 if(process_id>0){  
	 	//parent process
	 	this->parent_process(argc,argv);

	 }else if(process_id == 0){
	 	//child process

	 }
}


//start task
template<typename T>
void Process<T>::start(){
	if(process_id==0){ 
		//child process
		this->child_process();
	}
}


template<typename T>
int Process<T>::parent_process(int argc,char *argv[])
{
	int status;
	pid_t ret;

	ret = wait(&status);   //wait

	if(ret <0){
		perror("wait error");
		exit(EXIT_FAILURE);
	}
	//exit normal
	if (WIFEXITED(status)){
		printf("child exited normal exit status=%d\n", WEXITSTATUS(status));

	}

	//exit signal 
	else if (WIFSIGNALED(status)){
		printf("child exited abnormal signal number=%d\n", WTERMSIG(status));
	}

	//exit un normal
	else if (WIFSTOPPED(status)){
		printf("child stoped signal number=%d\n", WSTOPSIG(status));
		this->init(task,argc,argv);
	}
	
	return 0;
}

template<typename T>
void Process<T>::child_process()
{
	printf("child process is running\n");
	//run task
	this->task->run();
}

template<typename T>
Process<T>::~Process(){
	
}






#endif