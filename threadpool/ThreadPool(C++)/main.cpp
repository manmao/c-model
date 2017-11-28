#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/epoll.h>
#include <iostream>

#include "locker.h"
#include "threadpool.h"

using namespace std;

class Test{
private:

public:
	 void process();
};

void Test::process()
{
	cout<<"Hello world"<<endl;
}


int main(int argc,char *argv[])
{
	Test *test=new Test();

	threadpool<Test> *th=new threadpool<Test>(10,100000);
	th->append(test);

	sleep(2);
	delete test;
	delete th;
	return 0;

}
