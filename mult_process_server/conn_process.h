#ifndef __CONN_PROCESS_H
#define __CONN_PROCESS_H

#include "common.h"
#include "context.h"

#ifdef __cplusplus
 extern "C"{
#endif

	void process_accept(struct conn_context * listen_ctx,struct worker_data *wdata);

	void process_close(struct conn_context *ctx);

#ifdef __cplusplus
} 
#endif

#endif