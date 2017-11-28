#include <assert.h>
#include "context.h"


struct context_pool *init_pool(int size)
{
	struct context_pool *ret;
	int i;

	assert(size > 0);

	ret = malloc(sizeof(struct context_pool));
	assert(ret);

	ret->arr = malloc(sizeof(struct conn_context) * size);
	assert(ret->arr);

	ret->total = size;
	ret->allocated = 0;
	ret->next_idx = 0;

	for (i = 0; i < size - 1; i++)
		ret->arr[i].next_idx = i + 1;

	ret->arr[size - 1].next_idx = -1;

	return ret;
}

struct conn_context *alloc_context(struct context_pool *pool)
{
	struct conn_context *ret;

	assert(pool->allocated < pool->total);
	pool->allocated++;

	ret = &pool->arr[pool->next_idx];
	pool->next_idx = pool->arr[pool->next_idx].next_idx;

	ret->fd = 0;
	ret->fd_added = 0;
	ret->end_fd = 0;
	ret->end_fd_added = 0;
	ret->next_idx = -1;

	ret->pool = pool;

	return ret;
}

void free_context(struct conn_context *context)
{
	struct context_pool *pool = context->pool;

	assert(pool->allocated > 0);
	pool->allocated--;

	context->next_idx = pool->next_idx;
	pool->next_idx = context - pool->arr;
}
