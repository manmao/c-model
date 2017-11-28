#include <stdio.h>
#include <unistd.h>
#include "time_wheel.h"

void cb_fun(client_data *data)
{
	printf("timeout\n");
}

int main(int argc,char *argv[])
{
	time_wheel *tw=new time_wheel();
	tw_timer *tt=tw->add_timer(1,&cb_fun,NULL);
	tw_timer *tt2=tw->add_timer(5,&cb_fun,NULL);
	tw_timer *tt3=tw->add_timer(7,&cb_fun,NULL);
	tw->del_timer(tt2);

	while(1)
	{
		sleep(1); 
		printf("\n");
		tw->tick();
		
	}


	return 0;
}