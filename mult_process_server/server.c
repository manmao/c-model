/*
 * server.c
 *
 * Copyright (C) SINA Corporation
 *
 * Simple HTTP server that can be used in web server mode
 * and proxy server mode.
 * It is used to test TCP netwotk stack performance.
 *
 */

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

#include "server.h"
#include "common.h"
#include "context.h"
#include "conn_process.h"


struct worker_data *wdata;
int num_workers;      //工作进程的个数
int start_cpu = 0;  //第一个开始绑定的CPU

int enable_verbose = 0;
int enable_proxy = 0; //是否开启代理
int enable_debug = 0;  //是否开启debug模式
int process_mode = 0;  //是否开启多进程模式

int specified_log_file = 0;
FILE *log_file = NULL;
int pfd;

//服务器监听地址个数
int la_num;
struct listen_addr la[MAX_LISTEN_ADDRESS];

//服务器代理的后端服务器的个数
int pa_num;
struct proxy_addr pa[MAX_PROXY_ADDRESS];

char log_path[64] = {0};

/**
 * [获取CPU核心数]
 * @return [description]
 */
static int get_cpu_num()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

/**
 * 进程绑定CPU
 * @param  cpu [description]
 * @return     [description]
 */
static int bind_process_cpu(int cpu)
{
	cpu_set_t cmask;
	size_t n;
	int ret;

	n = get_cpu_num();

	if (cpu < 0 || cpu >= (int)n) {
		errno = EINVAL;
		return -1;
	}

	CPU_ZERO(&cmask);
	CPU_SET(cpu, &cmask);

	ret = sched_setaffinity(0, n, &cmask);

	CPU_ZERO(&cmask);

	return ret;
}


/**
 * Usage: ./bin  [-d] [-c] [start_cpu] [-w] [worker_num] [-a] [ip:port ...] [-x] [ip:port ...]
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main(int argc, char *argv[])
{

	printf("Usage: %s [-d] [-c] [start_cpu] [-w] [worker_num] [-a] [ip:port ...] [-x] [ip:port ...]\n",
	       argv[0]);
	printf("   -c: specify the first cpu to bind each worker\n	   [default is 0]\n");
	printf("   -w: specify worker number\n	   [default is the available cpu core number]\n");
	printf("   -a: specify frontend listen address\n	   [default is 0.0.0.0:80]\n");
	printf("   -x: enable proxy mode and specify backend listen address\n	   [default is off]\n");
	printf("   -v: enable verbose mode\n	   [default is off]\n");
	printf("   -d: enable debug mode\n	   [default is off]\n");
	printf("   -o: specify log file\n	   [default is ./demo.log]\n");
	printf("\n");

again:
	if (argc >= 2 && strcmp(argv[1], "-d") == 0) {
		enable_debug = 1;
		argv++;
		argc--;
		goto again;
	}

	if (argc >= 2 && strcmp(argv[1], "-v") == 0) {
		enable_verbose = 1;
		argv++;
		argc--;
		goto again;
	}

	if (argc >= 3 && strcmp(argv[1], "-o") == 0) {
		strncpy(log_path, argv[2], sizeof(log_path));
		specified_log_file = 1;
		argv += 2;
		argc -= 2;
		goto again;
	}

	if (argc >= 3 && strcmp(argv[1], "-c") == 0) {
		start_cpu = atoi(argv[2]);

		argv += 2;
		argc -= 2;
		goto again;
	}

	if (argc >= 3 && strcmp(argv[1], "-w") == 0) {
		process_mode = 1;
		num_workers = atoi(argv[2]);

		argv += 2;
		argc -= 2;
		goto again;
	}

	if (argc >= 3 && strcmp(argv[1], "-a") == 0) {
		int i ;

		for (i = 0; i < MAX_LISTEN_ADDRESS && argv[2]; i++) {
			char *sep = strchr(argv[2], ':');

			if (sep) {
				*sep = 0;
				strncpy(la[i].param_ip, argv[2], 32);//赋值IP地址
				inet_aton(la[i].param_ip, &la[i].listenip); 
				sscanf(++sep, "%d", &la[i].param_port); //获取port
			} else
				break;
			argv++;
			argc--;
			la_num++;
		}

		argv++;
		argc--;
		goto again;
	}

	if (argc >= 3 && strcmp(argv[1], "-x") == 0) {
		int i ;

		enable_proxy = 1;

		for (i = 0; i < MAX_PROXY_ADDRESS && argv[2]; i++) {
			char *sep = strchr(argv[2], ':');

			if (sep) {
				*sep = 0;
				strncpy(pa[i].param_ip, argv[2], 32);
				inet_aton(pa[i].param_ip, &pa[i].proxyip);
				sscanf(++sep, "%d", &pa[i].param_port);
			} else
				break;

			argv++;
			argc--;
			pa_num++;
		}

		argv++;
		argc--;
		goto again;
	}

	if (!process_mode)
		process_mode = 1;
	if (!num_workers)
		num_workers = get_cpu_num();

	assert(num_workers >= 1 && num_workers <= get_cpu_num());

	//服务IP:PORT
	if (la_num) {  //自定义	ip:端口
		int i;
		printf("Specified listen address:\n");
		for (i = 0; i < la_num; i++) {
			printf("\t%s:%d\n",la[i].param_ip, la[i].param_port);
		}
	} else {    //默认打开80端口
		la_num = 1;
		strncpy(la[0].param_ip, "0.0.0.0", 32);
		inet_aton(la[0].param_ip, &la[0].listenip);
		la[0].param_port = 80;
		printf("Default listen address:\n\t%s:%d\n",la[0].param_ip, la[0].param_port);
	}
	printf("\n");

	//反向代理服务器地址
	if (pa_num) {  //代理模式
		int i;
		printf("Proxy mode is enabled, back-end address:\n");
		for (i = 0; i < pa_num; i++) {
			printf("\t%s:%d\n",pa[i].param_ip, pa[i].param_port);
		}
		printf("\n");
	}

	if (enable_debug)
		printf("Debug Mode is enabled\n\n");

	if (process_mode)
		printf("Process Mode is enable with %d workers\n\n", num_workers);

	init_log();  	//初始化日志
	init_server();  //初始化服务器socket和系统核心配置
	init_signal();  //初始化信号
	init_workers(); //初始化工作进程
	init_timer();  //初始化定时器
	do_stats();   //处理信号

	return 0;
}

void init_signal(void)
{
	sigset_t siglist;

	if(sigemptyset(&siglist) == -1) {
		perror("Unable to initialize signal list");
		exit_cleanup();
	}

	if(sigaddset(&siglist, SIGALRM) == -1) {
		perror("Unable to add SIGALRM signal to signal list");
		exit_cleanup();
	}

	if(sigaddset(&siglist, SIGINT) == -1) {
		perror("Unable to add SIGINT signal to signal list");
		exit_cleanup();
	}

	if(pthread_sigmask(SIG_BLOCK, &siglist, NULL) != 0) {
		perror("Unable to change signal mask");
		exit_cleanup();
	}
}

void init_timer(void)
{
	struct itimerval interval;

	interval.it_interval.tv_sec = 1;
	interval.it_interval.tv_usec = 0;
	interval.it_value.tv_sec = 1;
	interval.it_value.tv_usec = 0;
	//设置定时器,当定时到达时，会向该进程发送SIGALRM信号，通过捕捉SIGALRM信号来实现定时任务
	if(setitimer(ITIMER_REAL, &interval, NULL) != 0) {
		perror("Unable to set interval timer");
		exit_cleanup();
	}
}



void init_log(void)
{
	if (specified_log_file) {
		log_file = fopen(log_path, "a");
		if (!log_file)
			perror("Open log file failed");
		printf("Using specified log file %s\n\n", log_path);
	}

	if (!log_file) {
		log_file = fopen("demo.log", "a");
		if (!log_file) {
			perror("Open log file failed");
			exit_cleanup();
		} else {
			printf("Using default log file %s\n\n", "./demo.log");
		}
	}

	pfd = dup(STDERR_FILENO);

	dup2(fileno(log_file), STDOUT_FILENO);
	dup2(fileno(log_file), STDERR_FILENO);

	print_d("Log starts\n");
}


/**
 * 创建服务器监听套接字
 * @param  ip   [description]
 * @param  port [description]
 * @return      [description]
 */
static int init_single_server(struct in_addr ip, uint16_t port)
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int serverfd, flags, value;

	if((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Unable to open socket");
		exit_cleanup();
	}

	flags = fcntl(serverfd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(serverfd, F_SETFL, flags);

	value = 1;
	if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) ==
	   -1) {
		perror("Unable to set socket reuseaddr option");
		exit_cleanup();
	}

	memset(&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = ip;

	if(bind(serverfd, (struct sockaddr *)&addr, addrlen) == -1) {
		perror("Unable to bind socket");
		exit_cleanup();
	}

	if(listen(serverfd, 8192) != 0) {
		perror("Cannot listen for client connections");
		exit_cleanup();
	}

	return serverfd;
}


/**
 * 初始化服务器资源，创建套接字监听接口
 */
void init_server(void)
{
	int ret, i;
	struct rlimit limits;

	for (i = 0; i < la_num; i++){
		struct in_addr ip;
		uint16_t port;

		ip = la[i].listenip;
		port = la[i].param_port;

		la[i].listen_fd = init_single_server(ip, port);
	}

	limits.rlim_cur = RLIM_INFINITY;
	limits.rlim_max = RLIM_INFINITY;

	ret = setrlimit(RLIMIT_CORE, &limits);
	if (ret < 0) {
		perror("Set core limit failed");
		exit_cleanup();
	}

	getrlimit(RLIMIT_CORE, &limits);
	print_d("Core limit %ld %ld\n", limits.rlim_cur, limits.rlim_max);

	limits.rlim_cur = MAX_CONNS_PER_WORKER;
	limits.rlim_max = MAX_CONNS_PER_WORKER;

	ret = setrlimit(RLIMIT_NOFILE, &limits);
	if (ret < 0) {
		perror("Set open file limit failed");
		exit_cleanup();
	}

	getrlimit(RLIMIT_NOFILE, &limits);
	print_d("Open file limit %ld %ld\n", limits.rlim_cur, limits.rlim_max);
}




void init_workers(void)
{
	if (process_mode)
		init_processes();
}
/**
 * 初始化，开辟子进程
 */
void init_processes(void)
{
	int i, pid;

	wdata = mmap(NULL, num_workers * sizeof(struct worker_data),
		     PROT_READ|PROT_WRITE,
		     MAP_ANON|MAP_SHARED,
		     -1, 0);

	memset(wdata, 0, num_workers * sizeof(struct worker_data));

	if (wdata == NULL) {
		perror("Unable to mmap shared global wdata");
		exit_cleanup();
	}

	for(i = 0; i < num_workers; i++) {
		wdata[i].trancnt = 0;
		wdata[i].cpu_id = i + start_cpu;

		if ( (pid = fork()) < 0) {
			perror("Unable to fork child process");
			exit_cleanup();
		} else if( pid == 0) {
			wdata[i].process = getpid();
			process_clients((void *)&(wdata[i]));  //每个进程都去监听同一个端口的socket fd
			exit(0);
		}
	}
}




/**
 * [子进程执行单元]
 * @param  arg [description]
 * @return     [description]
 */
void *process_clients(void *arg)
{
	int ret;
	struct worker_data *mydata = (struct worker_data *)arg;
	struct context_pool *pool;

	struct epoll_event evt;
	struct epoll_event evts[EVENTS_PER_BATCH];

	int cpu_id;
	int ep_fd;
	int i;

	struct conn_context *ctx;

	
	ret = bind_process_cpu(mydata->cpu_id);
	if (ret < 0) {
		perror("Unable to Bind worker on CPU");
		exit_cleanup();
	}

	pool = init_pool(MAX_CONNS_PER_WORKER);

	/**
	 * 创建epoll实例
	 */
	if ((ep_fd = epoll_create(MAX_CONNS_PER_WORKER)) < 0) {
		perror("Unable to create epoll FD");
		exit_cleanup();
	}

	/**
	 * 将多个la[i].listen_fd添加到epoll
	 */
	for (i = 0; i < la_num; i++) {
		ctx = alloc_context(pool);

		ctx->fd = la[i].listen_fd;  //socket监听端口
		ctx->handler = process_accept; //绑定epoll事件处理函数
		cpu_id = mydata->cpu_id;
		ctx->cpu_id = cpu_id;
		ctx->ep_fd = ep_fd;
		//代理信息
		ctx->enable_proxy=enable_proxy;
		if(ctx->enable_proxy){ 
			int i=0;
			ctx->pa_num=pa_num;
			for(i=0;i<pa_num;i++){
				ctx->pa[i]=pa[i];
			}
		}
		
		evt.events = EPOLLIN | EPOLLHUP | EPOLLERR;
		evt.data.ptr = ctx;  //将ctx放到epoll epoll_event的data.ptr指针

		if (epoll_ctl(ctx->ep_fd, EPOLL_CTL_ADD, ctx->fd, &evt) < 0) {
			perror("Unable to add Listen Socket to epoll");
			exit_cleanup();
		}
	}

	wdata[cpu_id].polls_min = EVENTS_PER_BATCH;

	//进入epoll是事件循环处理,当有事件时
	while (1) {
		int num_events;
		int i;
		int events;

		num_events = epoll_wait(ep_fd, evts, EVENTS_PER_BATCH, -1);
		if (num_events < 0) {
			if (errno == EINTR)
				continue;
			perror("epoll_wait() error");
		}
		if (!num_events)
			wdata[cpu_id].polls_mpt++;
		else if (num_events < wdata[cpu_id].polls_min)
			wdata[cpu_id].polls_min = num_events;
		if (num_events > wdata[cpu_id].polls_max)
			wdata[cpu_id].polls_max = num_events;

		wdata[cpu_id].polls_sum += num_events;
		wdata[cpu_id].polls_cnt++;
		wdata[cpu_id].polls_avg = wdata[cpu_id].polls_sum / wdata[cpu_id].polls_cnt;
		wdata[cpu_id].polls_lst = num_events;

		for (i = 0 ; i < num_events; i++) {  //处理epoll事件
			int active_fd;

			events = evts[i].events;
			ctx = evts[i].data.ptr;
			ctx->events = events;

			if (ctx->flags & PROXY_BACKEND_EVENT)
				active_fd = ctx->end_fd;
			else
				active_fd = ctx->fd;

			print_d("%dth event[0x%x] at fd %d\n", i, events, active_fd);
			//此处交由线程池处理
			ctx->handler(ctx,wdata);
		}
	}
	return NULL;
}


/**
 * 服务器信号处理操作
 * 包括定时器任务
 *  和终止信号
 */
void do_stats(void)
{
	sigset_t siglist;
	int signum;
	int i;

	FILE *p = fdopen(pfd, "w");

	if(sigemptyset(&siglist) == -1) {
		perror("Unable to initalize stats signal list");
		exit_cleanup();
	}

	if(sigaddset(&siglist, SIGALRM) == -1) {
		perror("Unable to add SIGALRM signal to stats signal list");
		exit_cleanup();
	}


	if(sigaddset(&siglist, SIGINT) == -1) {
		perror("Unable to add SIGINT signal to stats signal list");
		exit_cleanup();
	}

	while(1) {
		if(sigwait(&siglist, &signum) != 0) {
			perror("Error waiting for signal");
			exit_cleanup();
		}

		if(signum == SIGALRM) {
			uint64_t trancnt = 0;

			for(i = 0; i < num_workers; i++)
			{
				trancnt += wdata[i].trancnt - wdata[i].trancnt_prev;
				if (enable_verbose)
					fprintf(p, "%"PRIu64"[%"PRIu64"-%"PRIu64"-%"PRIu64"-%"PRIu64"-%"PRIu64"-%"PRIu64"-%"PRIu64"-%"PRIu64"]  ",
						wdata[i].trancnt - wdata[i].trancnt_prev, wdata[i].polls_mpt,
						wdata[i].polls_lst, wdata[i].polls_min, wdata[i].polls_max,
						wdata[i].polls_avg, wdata[i].accept_cnt, wdata[i].read_cnt,
						wdata[i].write_cnt);
				wdata[i].trancnt_prev = wdata[i].trancnt;
			}

			fprintf(p, "\tRequest/s %8"PRIu64"\n", trancnt);

		} else if(signum == SIGINT) {
			stop_workers();
			break;
		}
	}
}

void exit_cleanup(void)
{
	stop_workers();
	exit(EXIT_FAILURE);
}

void stop_processes(void)
{
	int i;

	if (wdata) {
		for(i = 0; i < num_workers; i++) {
			if (wdata[i].process)
				kill(wdata[i].process, SIGTERM); //向每个进程发送关闭信号
		}
	}
}

void stop_workers(void)
{
	if (process_mode)
		stop_processes();
}