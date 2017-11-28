#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"

void task_fun (void *arg)
{
    long id = (long)arg;
    int n;
    for (n = 0; n < 10; n++) {
        printf("thread %d (%d)\n", id, n);
        sleep(1);
    }
}

int main()
{
    int i;


    ThreadPool * tpool = threadpool_create(3, 10);


    for (i = 0;  i < 6; i++) {
        if (threadpool_add_task(tpool, task_fun, (void *)i) != 0) {
            printf("threadpool_add_task error\n");
        }
    }


    sleep(120);


   // threadpool_stop_all_task(tpool);
   // threadpool_destroy(tpool);
    return 0;
}