#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sched.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>

#include "common.h"
#include "context.h"

#include "conn_process.h"

#include "web_server.h"
#include "proxy_server.h"

/**
 * 处理连接,
 * @param listen_ctx [description]
 */
void process_accept(struct conn_context * listen_ctx,struct worker_data *wdata)
{
	int client_fd, listen_fd;
	int events = listen_ctx->events;
	struct epoll_event evt;

	struct context_pool *pool;
	struct conn_context *client_ctx;

	int cpu_id = listen_ctx->cpu_id;
	int ret = 0;
	int i;

	listen_fd = listen_ctx->fd;

	//TODO: What else should I do.
	if (events & (EPOLLHUP | EPOLLERR))
		return;

	for (i = 0; i < ACCEPT_PER_LISTEN_EVENT; i++) {
		int flags;

		client_fd = accept(listen_fd, NULL, NULL);
		if (client_fd < 0) {
			wdata[cpu_id].accept_cnt++;
			goto back;
		}

		flags = fcntl(client_fd, F_GETFL, 0);
		flags |= O_NONBLOCK;
		fcntl(client_fd, F_SETFL, flags);

		print_d("Accept socket %d from %d\n", client_fd, listen_fd);
	}

	pool = listen_ctx->pool;
	client_ctx = alloc_context(pool);
	assert(client_ctx);
	//复制Context信息
	client_ctx->fd = client_fd;

	if (listen_ctx->enable_proxy){  //代理服务程序
		client_ctx->handler = process_read_frontend; //绑定读取数据函数

		//代理服务器信息
		client_ctx->enable_proxy=listen_ctx->enable_proxy;
		if(client_ctx->enable_proxy){
			int i=0;
			client_ctx->pa_num=listen_ctx->pa_num;
			for(i=0;i<client_ctx->pa_num;i++){
				client_ctx->pa[i]=listen_ctx->pa[i];
			}
		}
	}else{                      	//普通服务程序
		client_ctx->handler = process_read; 	//绑定读取数据函数
	}

	client_ctx->cpu_id = listen_ctx->cpu_id;
	client_ctx->ep_fd = listen_ctx->ep_fd;

	
	evt.events = EPOLLIN | EPOLLHUP | EPOLLERR;
	evt.data.ptr = client_ctx;

	//添加到epoll监听
	ret = epoll_ctl(client_ctx->ep_fd, EPOLL_CTL_ADD, client_ctx->fd, &evt);
	if (ret < 0) {
		perror("Unable to add client socket read event to epoll");
		goto free_back;
	}

	client_ctx->fd_added = 1;

	goto back;

free_back:

	print_d("cpu[%d] close socket %d\n", cpu_id, client_ctx->fd);

	process_close(client_ctx);
	free_context(client_ctx);
back:
	return;
}


/**
 * 处理关闭连接
 * @param ctx [description]
 */
void process_close(struct conn_context *ctx)
{
	int fd, end_fd, ep_fd, ret;
	struct epoll_event evt;

	ep_fd = ctx->ep_fd;
	fd = ctx->fd;
	end_fd = ctx->end_fd;

	evt.events = EPOLLHUP | EPOLLERR;
	evt.data.ptr = ctx;


	if (ctx->fd_added) {
		ret = epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, &evt);
		if (ret < 0)
			perror("Unable to delete client socket from epoll");
	}
	close(fd);

	if (end_fd) {
		if (ctx->end_fd_added) {
			ret = epoll_ctl(ep_fd, EPOLL_CTL_DEL, end_fd, &evt);
			if (ret < 0)
				perror("Unable to delete client socket from epoll");
		}
		close(end_fd);
	}
}