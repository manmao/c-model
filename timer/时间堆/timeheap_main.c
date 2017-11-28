#include <stdio.h>
#include <unistd.h>
#include "time_heap.h"

void cb_fun(client_data *data)
{
	printf("timeout\n");
}

int main(int argc,char *argv[])
{

	time_heap *th=new time_heap(10);

	heap_timer *htimer=new heap_timer(3);
	htimer->cb_func=&cb_fun;
	htimer->user_data=NULL;

	heap_timer *htimer1=new heap_timer(4);
	htimer1->cb_func=&cb_fun;
	htimer1->user_data=NULL;

	th->add_timer(htimer);
	th->add_timer(htimer1);


	while(1)
	{
		sleep(1);
		printf("time\n");
		th->tick();
	}

	return 0;
}