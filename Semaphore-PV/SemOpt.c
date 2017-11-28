#include <stdio.h>
#include <errno.h>
#include "SemOpt.h"
///建立信号量，魔数key和信号量的初始值value;

sem_t CreateSem(key_t  key,int value){

    union semun sem;

    sem_t semid;
    sem.val=value;

    semid=semget(key,1,IPC_CREAT|0666);

    if(-1 == semid){
        printf("create semaphore error\n");
        return -1;
    }

    semctl(semid,0,SETVAL,sem);

    return semid;
}

/*****************
struct sembuf{
    ushort sem_num;  ///信号量的编号
    short sem_op;  ///信号量的操作
    short sem_flg; ///信号量操作标志
};
*******************/

int Sem_P(sem_t semid){
       struct sembuf sops={0,+1,IPC_NOWAIT};
       return (semop(semid,&sops,1));
}

int Sem_V(sem_t semid){
    struct sembuf sops={0,-1,IPC_NOWAIT};
    return (semop(semid,&sops,1));
}


void SetvalueSem(sem_t semid,int value){
    union semun sem;
    sem.val=value;
    semctl(semid,0,SETVAL,sem);
}


int GetvalueSem(sem_t semid){
    union semun sem;
    return semctl(semid,0,GETVAL,sem);
}


void DestroySem(sem_t semid){
    union semun sem;
    sem.val=0;
    semctl(semid,0,IPC_RMID,sem);
}





