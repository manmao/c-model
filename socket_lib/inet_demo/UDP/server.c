#include <stdio.h>
#include <sys/socket.h>
#include "inet_sockets.h"

int main(int argc,char *argv[]){
    int fd;
    char buff[2014];
    fd=inetBind("8080",SOCK_DGRAM, NULL);
     printf("server waiting: \n");


    while(1){
      if(fd>0){
          int size=read(fd,buff,1024);
          if(size >0){
             printf("%s",buff);
          }
      }
    }
    return 0;
}