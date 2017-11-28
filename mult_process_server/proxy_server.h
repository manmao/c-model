#ifndef __PROXY_SERVER_H
#define __PROXY_SERVER_H

#include "common.h"
#include "context.h"


#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * [process_read_backend 获取后端服务器传送过来数据]
	 * @param ctx []
	 */
	 void process_read_backend(struct conn_context *ctx,struct worker_data *wdata);

	 /**
	  * [process_write_backend 发送数据给后端服务器]
	  * @param ctx [description]
	  */
	 void process_write_backend(struct conn_context *ctx,struct worker_data *wdata);
	 
	 /**
	  * [process_read_frontend 获取前端服务器传送过来的数据]
	  * @param ctx [description]
	  */
	 void process_read_frontend(struct conn_context *ctx,struct worker_data *wdata);

	 /**
	  * [process_write_frontend 发送数据给前端服务器]
	  * @param ctx [description]
	  */
	 void process_write_frontend(struct conn_context *ctx,struct worker_data *wdata);

#ifdef __cplusplus
 }
#endif

#endif