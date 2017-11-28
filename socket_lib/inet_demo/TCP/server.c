#include <stdio.h>
#include <sys/socket.h>
#include "inet_sockets.h"

int main(int argc,char *argv[]){
    int fd;
    char buff[2014];
    fd=inetListen("8080",5, NULL);

    printf("server waiting: \n");

  // 可用epoll标签
    int afd=inetAccept(fd,"8080",NULL);

    while(1){
      if(afd>0){
          int size=read(afd,buff,1024);
          if(size >0){
             printf("%s",buff);
          }
      }
    }
    return 0;
}