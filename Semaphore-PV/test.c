#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "SemOpt.h"


int main(int argc,char *argv[]){

    key_t key,key1;
    int semid,semid1;
    char i;
    int value=0;

    //key=ftok(argv[1],'a');
    //key1=ftok(argv[2],'a');

    semid=CreateSem(0x1234,1); //
    semid1=CreateSem(0x5678,0);

    int pid=fork();

    if(pid < 0){
        return -1;
    }

    if(pid >0){
        int i=0;
        while(1){
            int value=0;

           // while((value=GetvalueSem(semid))<=0);
            Sem_P(semid1);　//如果小于等于0,阻塞
                printf("Parent %d\n",i++);
            Sem_V(semid);
        }

    }
    if(pid==0){
         int i=0;
        while(1){

            int value=0;
           // while((value=GetvalueSem(semid1))<=0);
            Sem_P(semid); //如果小于等于0,阻塞
                printf("child %d\n",i++);
            Sem_V(semid1);


        }

    }

    DestroySem(semid);
    DestroySem(semid1);
    return 0;
}
