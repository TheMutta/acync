#include "../include/async.h"

#include <assert.h>
#include <stdlib.h>

// worker thread function
static void *worker_thread(void *arg) {
	async_runtime runtime = (async_runtime)arg;
	async_task_queue *queue = &runtime->queue;

	while(runtime->active) {
		pthread_mutex_lock(&queue->lock);

		// while queue empty, wait
		while(queue->in == queue->work_index) {
			pthread_cond_wait(&queue->not_empty, &queue->lock);

			if (!runtime->active) {
				pthread_mutex_unlock(&queue->lock);
				return NULL;
			}
		}

		// gets the task to execute
		async_task *task = queue->data[queue->work_index & queue->mask];
		++queue->work_index;

		pthread_mutex_unlock(&queue->lock);

		// runs the task
		task->ret = task->callback(task->arg);

		task->done = true;
		
		// signal task is done
		pthread_mutex_lock(&queue->lock);
		pthread_cond_signal(&queue->task_done);
		pthread_mutex_unlock(&queue->lock);
	}

	return NULL;
}

async_runtime async_create_runtime(uint16_t thread_count) {
	assert(thread_count != 0); // can't make a runtime with no threads right now

	async_runtime runtime = (async_runtime)malloc(sizeof(async_runtime_core));
	assert(runtime != NULL);

	// initialize the queue
	async_task_queue *queue = &runtime->queue;
	queue->size = 65536; // chosen statically for now
	queue->mask = queue->size - 1; // following the lead of kfifo
	queue->in = queue->out = queue->work_index = 0;
	queue->data = (async_task**)malloc(queue->size * sizeof(async_task*));
	assert(queue->data != NULL);
	pthread_mutex_init(&queue->lock, NULL);
	pthread_cond_init(&queue->not_empty, NULL);
	pthread_cond_init(&queue->task_done, NULL);

	// setting up the thread pool
	runtime->active = true;
	runtime->thread_count = thread_count;
	runtime->thread_pool = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
	assert(runtime->thread_pool != NULL);

	// strating the thread pool
	for (uint16_t thread = 0; thread < thread_count; ++thread) {
		int ret = pthread_create(&runtime->thread_pool[thread], NULL, worker_thread, (void*)runtime);
		assert(ret == 0);
	}

	return runtime;
}


void async_destroy_runtime(async_runtime runtime) {
	assert(runtime != NULL);

	// disable runtime
	runtime->active = false;

	async_task_queue *queue = &runtime->queue;
	for (uint16_t thread = 0; thread < runtime->thread_count; ++thread) {
		// wake all threads
		pthread_mutex_lock(&queue->lock);
		pthread_cond_broadcast(&queue->not_empty);
		pthread_mutex_unlock(&queue->lock);

		// reap threads
		int ret = pthread_join(runtime->thread_pool[thread], NULL);
		assert(ret == 0);
	}


	pthread_mutex_lock(&queue->lock);

	// free the queue
	queue->size = 0;
	queue->mask = 0;
	queue->in = queue->out = queue->work_index = 0;
	free(queue->data);

	// task structs are leaked

	// free the thread pool
	free(runtime->thread_pool);

	// free the runtime
	free(runtime);
}


async_future async_dispatch(async_runtime runtime, async_callback function, void *arg) {
	assert(runtime != NULL);

	async_task_queue *queue = &runtime->queue;

	pthread_mutex_lock(&queue->lock);

	// check if the queue is full
	if ((queue->in - queue->out) == queue->size) {
		pthread_mutex_unlock(&queue->lock);
		assert(0 != 0);//panic, queue full
	}

	// create new task to dispatch
	async_task *task = (async_task*)malloc(sizeof(async_task));
	task->done = task->consumed = false;
	task->callback = function;
	task->arg = arg;
	task->id = queue->in;
	queue->data[queue->in & queue->mask] = task; // indexing trick
	++queue->in;

	// signaling that a new task is ready
	pthread_cond_signal(&queue->not_empty);
	pthread_mutex_unlock(&queue->lock);

	return task;
}

static void async_free_backlog(async_task_queue *queue) {
	// free as many queue slots as possible
	while (queue->in != queue->out) {
		async_task *oldest = queue->data[queue->out & queue->mask];

		if (oldest && oldest->done && oldest->consumed) {
			// if the oldest task is done and reaped, remove it
			queue->data[queue->out & queue->mask] = NULL;
			free(oldest);
			++queue->out;
		} else {
			// a task hasnt been reaped yet, tbd
			break;
		}
	}
}

async_result async_await(async_runtime runtime, async_future future) {
	assert(runtime != NULL);

	async_task_queue *queue = &runtime->queue;

	// wait for the task to be done
	async_task *task = future;
	pthread_mutex_lock(&queue->lock);
	while(!task->done) {
		pthread_cond_wait(&queue->task_done, &queue->lock);
	}

	// result is obtained
	async_result ret = task->ret;
	task->consumed = true;

	async_free_backlog(queue);

	pthread_mutex_unlock(&queue->lock);

	return ret;
}

async_result *async_await_many(async_runtime runtime, async_future *futures, size_t future_count) {
	// prepare the space for the results
	async_result *results = (async_result*)malloc(sizeof(async_result) * future_count);
	assert(results != NULL);

	for (size_t i = 0; i < future_count; ++i) {
		results[i] = async_await(runtime, futures[i]);
	}

	return results;
}

bool async_is_done(async_runtime runtime, async_future future, async_result *result) {
	assert(runtime != NULL);

	async_task *task = future;

	if(!task->done) {
		// if the task isnt done, just return
		return false;
	}
	
	// task is done, lock the queue and extract the value
	async_task_queue *queue = &runtime->queue;
	pthread_mutex_lock(&queue->lock);

	// result is obtained
	if (result) *result = task->ret;
	task->consumed = true;

	async_free_backlog(queue);

	pthread_mutex_unlock(&queue->lock);

	return true;
}
