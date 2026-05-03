#include "../include/async.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

async_result hello_world_async(async_arg arg) {
	printf("Hello, async %p!\n", arg);

	return (async_result)((uintptr_t)arg + 1);
}

async_result nop(async_arg arg) { return (async_result)arg; }

int main() {
	printf("creating runtime...\n");
	async_runtime runtime = async_create_runtime(2);
	printf("new runtime at %p\n", (void*)runtime);

	printf("dispatching...\n");

	// sync await test
	{
		async_future future1 = async_dispatch(runtime, hello_world_async, (async_arg)1);
		async_future future2 = async_dispatch(runtime, hello_world_async, (async_arg)2);
		async_future future3 = async_dispatch(runtime, hello_world_async, (async_arg)3);
		async_future future4 = async_dispatch(runtime, hello_world_async, (async_arg)4);

		async_future array[4] = {future1, future2, future3, future4};
		async_result *results = async_await_many(runtime, array, 4);

		printf("awaited. result1: %p\n", (void*)results[0]);
		printf("awaited. result2: %p\n", (void*)results[1]);
		printf("awaited. result3: %p\n", (void*)results[2]);
		printf("awaited. result4: %p\n", (void*)results[3]);

		free(results);
	}

	// sync await_many test
	{

		async_future future1 = async_dispatch(runtime, hello_world_async, (async_arg)1);
		async_future future2 = async_dispatch(runtime, hello_world_async, (async_arg)2);
		async_future future3 = async_dispatch(runtime, hello_world_async, (async_arg)3);
		async_future future4 = async_dispatch(runtime, hello_world_async, (async_arg)4);

		async_result res1 = async_await(runtime, future1);
		async_result res2 = async_await(runtime, future2);
		async_result res3 = async_await(runtime, future3);
		async_result res4 = async_await(runtime, future4);

		printf("awaited. result1: %p\n", (void*)res1);
		printf("awaited. result2: %p\n", (void*)res2);
		printf("awaited. result3: %p\n", (void*)res3);
		printf("awaited. result4: %p\n", (void*)res4);
	}

	// sync await runtime stress
	{
		for (size_t i = 0; i < 100000; i++) {
			async_future future = async_dispatch(runtime, nop, (async_arg)i);

			size_t result = (size_t)async_await(runtime, future);
			assert(result == i);
		}
	}

	printf("destroying runtime %p...\n", (void*)runtime);
	async_destroy_runtime(runtime);
	printf("bye\n");
}
