
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

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
#include "proxy_server.h"

/**
 * 从反向代理的服务器集群中选择一个服务器
 *  轮询方式
 * @param ctx  [description]
 * @param addr [description]
 */
static void select_backend(struct conn_context *ctx,struct sockaddr_in *addr)
{
	static int last;

	addr->sin_family = AF_INET;
	addr->sin_port = htons(ctx->pa[last].param_port);
	addr->sin_addr = ctx->pa[last].proxyip;

	print_d("Select Back-end server %s:%d\n", ctx->pa[last].param_ip,ctx->pa[last].param_port);

	last++;

	if (last == ctx->pa_num)
		last = 0;
}

void process_read_frontend(struct conn_context *ctx,struct worker_data *wdata)
{
	int ep_fd, front_fd, end_fd;
	char *buf = ctx->buf;
	int events = ctx->events;
	struct epoll_event evt;
	struct sockaddr_in addr_in;
	int ret;
	int cpu_id = ctx->cpu_id;

	ep_fd = ctx->ep_fd;
	front_fd = ctx->fd;

	//FIXME: What else should I do.
	if (events & (EPOLLHUP | EPOLLERR)) {
		printf("process_read_frontend() with events HUP or ERR\n");
		goto free_back;
	}

	print_d("Process read event[%02x] on front-end socket %d\n", events, front_fd);

	ret = read(front_fd, buf, MAX_BUFSIZE);
	if (ret < 0)
	{
		wdata[cpu_id].read_cnt++;
		perror("process_read_frontend() can't read client socket");
		goto free_back;
	}

	ctx->data_len = ret;

	print_d("Read %d from front-end socket %d\n", ret, front_fd);

	//Remove interested read event for front-end socket
	evt.events = EPOLLHUP | EPOLLERR;
	evt.data.ptr = ctx;

	ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, front_fd, &evt);
	if (ret < 0) {
		perror("Unable to add client socket read event to epoll");
		goto free_back;
	}

	int flags;

	ret = socket(AF_INET, SOCK_STREAM, 0);
	if (ret < 0) {
		perror("Unable to create new socket for backend");
		goto free_back;
	}

	end_fd = ret;
	ctx->end_fd = end_fd;

	print_d("Create socket %d\n", ret);

	flags = fcntl(ret, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(ret, F_SETFL, flags);

	struct linger ling = {1, 0};

	ret = setsockopt(end_fd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
	if (ret < 0) {
		perror("Unable to set socket linger option");
		goto free_back;
	}

	select_backend(ctx,&addr_in);

	ret = connect(end_fd, (struct sockaddr *)&addr_in, sizeof(struct sockaddr));
	if (ret < 0) {
		if (errno != EINPROGRESS) {
			perror("Unable to connect to back end server");
			goto free_back;
		}
	}

	ctx->handler = process_write_backend;  //绑定发送数据给后端服务器的函数
	ctx->flags |= PROXY_BACKEND_EVENT;

	evt.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
	evt.data.ptr = ctx;

	ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, end_fd, &evt);
	if (ret < 0) {
		perror("Unable to add client socket read event to epoll");
		goto free_back;
	}

	ctx->end_fd_added = 1;

	print_d("Add back-end socket %d to epoll\n", end_fd);

	goto back;

free_back:

	print_d("cpu[%d] close socket %d\n", cpu_id, ctx->fd);

	process_close(ctx);
	free_context(ctx);

back:
	return;
}

void process_write_backend(struct conn_context *ctx,struct worker_data *wdata)
{
	int ep_fd, end_fd;
	int events = ctx->events;
	char *buf;
	int data_len;
	int ret;

	ep_fd = ctx->ep_fd;
	end_fd = ctx->end_fd;
	buf = ctx->buf;
	data_len = ctx->data_len;

	print_d("Process write event[%02x] on back-end socket %d\n", events, end_fd);

	if (events & (EPOLLHUP | EPOLLERR)) {
		printf("process_write_backend() with events HUP or ERR 0x%x\n", events);
		goto free_back;
	}

	struct epoll_event evt;

	if (!(ctx->flags & PROXY_BACKEND_EVENT))
	{
		printf("Write to back-end socket while back end socket not enabled\n");
		goto free_back;
	}

	ret = write(end_fd, buf, data_len);
	if (ret < 0) {
		perror("process_write() can't write back end socket");
		goto free_back;
	}

	print_d("Write %d to back-end socket %d\n", ret, end_fd);

	ctx->handler = process_read_backend;
	ctx->flags |= PROXY_BACKEND_EVENT;

	evt.events = EPOLLIN | EPOLLHUP | EPOLLERR;
	evt.data.ptr = ctx;

	ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, end_fd, &evt);
	if (ret < 0) {
		perror("Unable to add client socket read event to epoll");
		goto free_back;
	}

	goto back;

free_back:

	process_close(ctx);
	free_context(ctx);

back:

	return;
}


void process_read_backend(struct conn_context *ctx,struct worker_data *wdata)
{
	int front_fd, end_fd, ep_fd;
	char *buf;
	int cpu_id;
	struct epoll_event evt;
	int ret;

	cpu_id = ctx->cpu_id;
	ep_fd = ctx->ep_fd;
	end_fd = ctx->end_fd;
	front_fd = ctx->fd;

	buf = ctx->buf;

	if (!(ctx->flags & PROXY_BACKEND_EVENT)) {
		printf("Process back end read while backend socket not enable\n");
		goto free_back;
	}

	ret = read(end_fd, buf, MAX_BUFSIZE);
	if (ret < 0) {
		wdata[cpu_id].read_cnt++;
		perror("process_read_backend() can't read client socket");
		goto free_back;
	}

	print_d("Read %d from back end socket %d\n", ret, end_fd);

	ctx->handler = process_write_frontend;
	ctx->flags &= ~PROXY_BACKEND_EVENT;
	ctx->data_len = ret;

	evt.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
	evt.data.ptr = ctx;

	ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, front_fd, &evt);
	if (ret < 0) {
		perror("Unable to add client socket read event to epoll");
		goto free_back;
	}

	evt.events = EPOLLHUP | EPOLLERR;
	evt.data.ptr = ctx;

	//FIXME: Why monitor end fd?
	ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, end_fd, &evt);
	if (ret < 0)
	{
		perror("Unable to add client socket read event to epoll");
		goto free_back;
	}

	goto back;

free_back:

	process_close(ctx);
	free_context(ctx);

back:

	return;
}




void process_write_frontend(struct conn_context *ctx,struct worker_data *wdata)
{
	int front_fd;
	char *buf;
	int data_len;
	int cpu_id;
	int ret;

	cpu_id = ctx->cpu_id;
	front_fd = ctx->fd;
	buf = ctx->buf;
	data_len = ctx->data_len;

	if (ctx->flags & PROXY_BACKEND_EVENT) {
		printf("Write to front end socket while back end socket enabled\n");
		goto free_back;
	}

	ret = write(front_fd, buf, data_len);
	if (ret < 0) {
		perror("Can't write front-end socket");
		goto free_back;
	}

	print_d("Write %d to front end socket %d\n", data_len, front_fd);

	wdata[cpu_id].trancnt++;

free_back:

	process_close(ctx);
	free_context(ctx);

	return;
}