#ifndef __ACYNC_ASYNC_H
#define __ACYNC_ASYNC_H

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <pthread.h>

typedef void* async_result;
typedef void* async_arg;
typedef async_result (*async_callback)(async_arg);

typedef struct {
	pthread_cond_t done_signal;
	_Atomic bool done;
	async_callback callback;
	async_arg arg;
	async_result ret;
} async_task;
typedef async_task *async_future;

typedef struct {
	async_task **data;
	size_t size;
	_Atomic size_t in;
	_Atomic size_t out;
	size_t mask;
	
	pthread_mutex_t lock;
	pthread_cond_t not_empty;
} async_task_queue;

typedef struct {
	bool active;

	async_task_queue queue;

	uint16_t thread_count;
	pthread_t *thread_pool;

} async_runtime_core;
typedef async_runtime_core* async_runtime;


async_runtime async_create_runtime(uint16_t thread_count);
void async_destroy_runtime(async_runtime runtime);
async_future async_dispatch(async_runtime runtime, async_callback function, void *arg); 
async_result async_await(async_runtime runtime, async_future future);

#endif /* __ACYNC_ASYNC_H */
