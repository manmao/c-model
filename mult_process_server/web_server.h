#ifndef __WEB_SERVER_H
#define __WEB_SERVER_H

#include "common.h"
#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif


void process_write(struct conn_context *client_ctx,struct worker_data *wdata);

void process_read(struct conn_context *client_ctx,struct worker_data *wdata);


#ifdef __cplusplus
}
#endif

#endif