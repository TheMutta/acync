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
	_Atomic bool done; // signal that the task has finished execution, therefore freeing the worker
	_Atomic bool consumed; // signal that the task has been consumed and is ready for freeing
	async_callback callback; // the worker function
	async_arg arg; // the arg for the worker function
	async_result ret; // the result of the returning callback function
	size_t id; // the id of the task
} async_task;
typedef async_task *async_future;

typedef struct {
	async_task **data; // queue data
	size_t size; // size of the queue in bytes
	_Atomic size_t in; // first free slot
	_Atomic size_t out; // first used slot
	_Atomic size_t work_index; // first slot that needs a worker
	size_t mask; // mask for the kfifo trick of infinite indexing
	
	pthread_mutex_t lock; // queue lock
	pthread_cond_t not_empty; // signal for the queue receiving new data
	pthread_cond_t task_done; // signal for a task being completed
} async_task_queue;

typedef struct {
	bool active; // the runtime is active
	async_task_queue queue; // the runtime queue

	uint16_t thread_count; // worker thread count
	pthread_t *thread_pool; // thw working threads

} async_runtime_core;
typedef async_runtime_core* async_runtime;

async_runtime async_create_runtime(uint16_t thread_count);
void async_destroy_runtime(async_runtime runtime);

async_future async_dispatch(async_runtime runtime, async_callback function, void *arg); 
async_result async_await(async_runtime runtime, async_future future);
bool async_is_done(async_runtime runtime, async_future future, async_result *result);

#endif /* __ACYNC_ASYNC_H */
