
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



char *http_200="HTTP/1.0 200 OK\r\n"
	"Cache-Control: no-cache\r\n"
	"Connection: close\r\n"
	"Content-Type: text/html\r\n"
	"\r\n"
	"<html><body><h1>200 OK</h1>\nEverything is fine.\n</body></html>\n";

char *http_response;
int http_response_len;


void process_write(struct conn_context *client_ctx,struct worker_data *wdata)
{
	int ep_fd, fd;
	int events = client_ctx->events;
	int cpu_id = client_ctx->cpu_id;
	int ret;
	struct epoll_event evt;

	ep_fd = client_ctx->ep_fd;
	fd = client_ctx->fd;

	print_d("Process write event[%02x]\n", events);

	if (events & (EPOLLHUP | EPOLLERR)) {
		printf("process_write() with events HUP or ERR\n");
		goto free_back;
	}

	http_response = http_200;
	http_response_len = strlen(http_response);

	ret = write(fd, http_response, http_response_len);
	if (ret < 0) {
		wdata[cpu_id].write_cnt++;
		perror("process_write() can't write client socket");
		goto free_back;
	}

	print_d("Write %d to socket %d\n", ret, fd);

	wdata[cpu_id].trancnt++;

	client_ctx->handler = process_read;

	evt.events = EPOLLIN | EPOLLHUP | EPOLLERR;
	evt.data.ptr = client_ctx;

	ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &evt);
	if (ret < 0) {
		perror("Unable to add client socket read event to epoll");
		goto free_back;
	}

	goto back;

free_back:

	process_close(client_ctx);
	free_context(client_ctx);
back:

	return;
}


void process_read(struct conn_context *client_ctx,struct worker_data *wdata)
{
	int ep_fd, fd;
	int events = client_ctx->events;
	struct epoll_event evt;
	int ret;
	char *buf = client_ctx->buf;
	int cpu_id = client_ctx->cpu_id;

	ep_fd = client_ctx->ep_fd;
	fd = client_ctx->fd;

	//FIXME: What else should I do.
	if (events & EPOLLHUP) {
		print_d("process_read() with events HUP\n");
		goto free_back;
	}
	if (events & EPOLLERR) {
		print_d("process_read() with events ERR\n");
		goto free_back;
	}

	print_d("Process read event[%02x] on socket %d\n", events, fd);

	ret = read(fd, buf, MAX_BUFSIZE);
	if (ret < 0) {
		wdata[cpu_id].read_cnt++;
		perror("process_read() can't read client socket");
		goto free_back;
	} else if (ret == 0) {
		goto free_back;
		print_d("Socket %d is closed\n", fd);
	}

	client_ctx->data_len = ret;

	print_d("Read %d from socket %d\n", ret, fd);

	client_ctx->handler = process_write;

	evt.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
	evt.data.ptr = client_ctx;

	ret = epoll_ctl(ep_fd, EPOLL_CTL_MOD, fd, &evt);
	if (ret < 0) {
		perror("Unable to add client socket read event to epoll");
		goto free_back;
	}
	
	goto back;

free_back:

	print_d("cpu[%d] close socket %d\n", cpu_id, client_ctx->fd);

	process_close(client_ctx);
	free_context(client_ctx);

back:
	return;
}
