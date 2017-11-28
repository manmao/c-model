#ifndef __CONTEXT_H
#define __CONTEXT_H


#include "common.h"


#ifdef __cplusplus
 extern "C"{
#endif
 	
struct context_pool {
	int total;
	int allocated;
	int next_idx;
	struct conn_context {
		int fd;        //客户端和服务器连接 FrontServer <-->ProxyServer
		int fd_added;  //是否对fd进行了赋值
		int end_fd;    //ProxyServer <--> BackServer 连接
		int end_fd_added;  // 是否添加了和反向服务器的连接
		int flags;
		int ep_fd;   //epoll fd
		int cpu_id;  //连接所在的cpu核心编号
		void (*handler)(struct conn_context *,struct worker_data *);
		int events;
		int data_len;
		struct context_pool *pool;
		int next_idx;
		char buf[MAX_BUFSIZE];
		//代理服务器需要使用的参数
		int pa_num;
		struct proxy_addr pa[MAX_PROXY_ADDRESS];
		int enable_proxy; //是否代理
	} *arr;
};

/**
 * 初始化上下文对象内存池
 * @param  size [description]
 * @return      [description]
 */
struct context_pool *init_pool(int size);

/**
 * 开辟一个上下文对象
 * @param  pool [description]
 * @return      [description]
 */
struct conn_context *alloc_context(struct context_pool *pool);


/**
 * 释放上下文对象
 * @param context [description]
 */
void free_context(struct conn_context *context);


#ifdef __cplusplus
} 
#endif

#endif