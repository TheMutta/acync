#include "../include/async.h"

#include <stdio.h>

async_result hello_world_async(async_arg arg) {
	printf("Hello, async %p!\n", arg);

	return (async_result)arg + 1;
}

int main() {
	printf("creating runtime...\n");
	async_runtime runtime = async_create_runtime(2);
	printf("new runtime at %p\n", runtime);

	printf("dispatching...\n");
	async_future future1 = async_dispatch(runtime, hello_world_async, (async_arg)1);
	async_future future2 = async_dispatch(runtime, hello_world_async, (async_arg)2);
	async_future future3 = async_dispatch(runtime, hello_world_async, (async_arg)3);
	async_future future4 = async_dispatch(runtime, hello_world_async, (async_arg)4);
	async_result res1 = async_await(runtime, future1);
	async_result res2 = async_await(runtime, future2);
	async_result res4 = async_await(runtime, future4);
	async_result res3 = async_await(runtime, future3);
	printf("awaited. result1: %p\n", res1);
	printf("awaited. result2: %p\n", res2);
	printf("awaited. result3: %p\n", res3);
	printf("awaited. result4: %p\n", res4);

	printf("destroying runtime %p...\n", runtime);
	async_destroy_runtime(runtime);
	printf("bye\n");
}
