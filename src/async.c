#include "../include/async.h"

#include <assert.h>
#include <stdlib.h>

static void *worker_thread(void *arg) {
	async_runtime runtime = (async_runtime)arg;
	async_task_queue *queue = &runtime->queue;

	while(true) {
		pthread_mutex_lock(&queue->lock);

		while(queue->in == queue->out) {
			pthread_cond_wait(&queue->not_empty, &queue->lock);
		}

		async_task *task = queue->data[queue->out & queue->mask];
		++queue->out;

		pthread_mutex_unlock(&queue->lock);

		task->ret = task->callback(task->arg);

		task->done = true;
	}

	return NULL;
}

async_runtime async_create_runtime(uint16_t thread_count) {
	assert(thread_count != 0); // can't make a runtime with no threads right now

	async_runtime runtime = (async_runtime)malloc(sizeof(async_runtime_core));
	assert(runtime != NULL);


	// initialize the queue
	async_task_queue *queue = &runtime->queue;
	queue->size = 1024 * 1024;
	queue->mask = queue->size - 1;
	queue->in = queue->out = 0;
	queue->data = (async_task**)malloc(queue->size / sizeof(async_task*));
	pthread_mutex_init(&queue->lock, NULL);


	runtime->active = true;
	runtime->thread_count = thread_count;
	runtime->thread_pool = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
	assert(runtime->thread_pool != NULL);

	for (uint16_t thread = 0; thread < thread_count; ++thread) {
		int ret = pthread_create(&runtime->thread_pool[thread], NULL, worker_thread, (void*)runtime);
		assert(ret == 0);

		ret = pthread_detach(runtime->thread_pool[thread]);
		assert(ret == 0);
	}

	return runtime;
}


void async_destroy_runtime(async_runtime runtime) {
	// todo
}


async_future async_dispatch(async_runtime runtime, async_callback function, void *arg) {
	async_task_queue *queue = &runtime->queue;

	pthread_mutex_lock(&queue->lock);

	if ((queue->in - queue->out) == queue->size) {
		pthread_mutex_unlock(&queue->lock);
		assert(0 != 0);//panic, queue full
	}

	async_task *task = (async_task*)malloc(sizeof(async_task));
	task->done = false;
	task->callback = function;
	task->arg = arg;
	queue->data[queue->in & queue->mask] = task;
	++queue->in;

	pthread_cond_signal(&queue->not_empty);
	pthread_mutex_unlock(&queue->lock);

	return task;
}

async_result async_await(async_runtime runtime, async_future future){
	async_task *task = future;
	while(!task->done) {

	}

	async_result ret = task->ret;
	free(task);

	return ret;
}
