#ifndef __COMMON_H
#define __COMMON_H

#include <inttypes.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MAX_CONNS_PER_WORKER    8192
#define MAX_BUFSIZE        2048

#define EVENTS_PER_BATCH	64
#define ACCEPT_PER_LISTEN_EVENT	1

#define MAX_LISTEN_ADDRESS	32
#define MAX_PROXY_ADDRESS	32

#define PROXY_BACKEND_EVENT	0x01

#define print_d(fmt, args...) ({\
		printf("\nWorker[%lu] %s:%d\t" fmt, syscall(__NR_gettid),__FUNCTION__ , __LINE__, ## args); \
	})

/**
 * 工作进程数据
 */
struct worker_data {
	pid_t process;
	uint64_t trancnt;
	uint64_t trancnt_prev;
	int cpu_id;
	uint64_t polls_max;
	uint64_t polls_min;
	uint64_t polls_avg;
	uint64_t polls_cnt;
	uint64_t polls_sum;
	uint64_t polls_mpt;
	uint64_t polls_lst;
	uint64_t accept_cnt;
	uint64_t read_cnt;
	uint64_t write_cnt;
};

struct listen_addr {
	int param_port;
	struct in_addr listenip;
	char param_ip[32];
	int listen_fd;//socket listen的地址的文件描述符
};

struct proxy_addr {
	int param_port;
	struct in_addr proxyip;
	char param_ip[32];
};


#endif