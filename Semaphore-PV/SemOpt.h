#ifndef __SEMOPT_H_
#define __SEMOPT_H_

/********************
union semun{            ///信号量操作的联合结构
    int val;
    struct semid_ds *buf;  //
    unsigned short *array;   //数组类型
    struct seminfo *__buf;   //信号量内部数据结构
}
**********************/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


typedef int sem_t;

union semun{
    int  val;
    struct  semid_ds *buf;
    unsigned short *array;
}arg;

///建立信号量，魔数key和信号量的初始值value;
//创建信号量
sem_t CreateSem(key_t  key,int value);

/*****************
struct sembuf{
    ushort sem_num;  ///信号量的编号
    short sem_op;  ///信号量的操作
    short sem_flg; ///信号量操作标志
};
*******************/

int Sem_P(sem_t semid);

int Sem_V(sem_t semid);


void SetvalueSem(sem_t semid,int value);


int  GetvalueSem(sem_t semid);


void DestroySem(sem_t semid);


#endif
