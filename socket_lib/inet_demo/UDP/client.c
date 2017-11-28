#include <stdio.h>
#include <sys/socket.h>
#include "inet_sockets.h"

int main(int argc,char *argv[]){
    int fd;
    char buff[2014];
    fd=inetConnect("127.0.0.1","8080",SOCK_DGRAM);

    while(1){
        write(fd,"Hee\n",100);
        usleep(400);
    }
    close(fd);
    return 0;
}










