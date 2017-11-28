#include <stdio.h>
#include "MainTask.h"

//
MainTask::MainTask(){
    printf("MainTask construct....\n")
}


//
MainTask::~MainTask(){
	
}


//init cmd
int MainTask::init(int argc,char *argv[]){
	printf("MainTask init....\n")
}

//
void MainTask::run(){
    printf("MainTask running......\n");
}

