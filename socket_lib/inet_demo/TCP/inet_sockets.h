/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2015.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* inet_sockets.h

   Header file for inet_sockets.c.
*/
#ifndef INET_SOCKETS_H
#define INET_SOCKETS_H          /* Prevent accidental double inclusion */

#include <sys/socket.h>
#include <netdb.h>

typedef int Bool;

#define TRUE 1
#define FALSE 0

//连接到指定的Ip:port,可以指定TCP或者UDP
//例子:
//   inetConnetc("127.0.0.1","8080",SOCK_STREAM); //建立一个TCP连接
int inetConnect(const char *host, const char *service, int type);

//用于TCP服务器bind端口
int inetListen(const char *service, int backlog, socklen_t *addrlen);

//通常用于UDP 服务器bind端口，type=SOCK_DGRM
int inetBind(const char *service, int type, socklen_t *addrlen);

//用于将sockaddr的地址转化成(hostname, port)
char *inetAddressStr(const struct sockaddr *addr, socklen_t addrlen,
                char *addrStr, int addrStrLen);

#define IS_ADDR_STR_LEN 4096
                        /* Suggested length for string buffer that caller
                           should pass to inetAddressStr(). Must be greater
                          than (NI_MAXHOST + NI_MAXSERV + 4) */

///用于TCP 接收客户端的连接
int inetAccept(int sockfd,const char *service, socklen_t *addrlen);

#endif
