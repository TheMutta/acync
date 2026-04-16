# aCync
## A C async ruintime

Useful library to add a simple and lightweight async runtime in your C program.

### Usage
First, include the `async.h` header in your file.
Then you can create the runtime with the following function:
```
async_runtime runtime = async_create_runtime(/* the amount of worker threads you desire */);
```
Afterwards, dispatching a task can be done simply with:

```
async_future future = async_dispatch(runtime, /* your async function */, /* the arg to send to the async function */);
```

Reaping the future can be done either in a syncronous or asyncronous way with the following functions:
```
/* sync */
async_result result = async_await(runtime, future);
/* async */
async_result result;
bool is_done = async_is_done(runtime, future, &result);
```

And finally, destroying the runtime is not needed at the end of the program, but if necessary you can simply do:
```
async_destroy_runtime(runtime);
```

### Building
To includ aCync in your project, you can either use CMake's fetch content or compile manually.

#### FetchContent
In your root `CMakeLists.txt` add:
```
include(FetchContent)

FetchContent_Declare(
  acync
  GIT_REPOSITORY https://gitlab.com/TheMutta/acync.git
  GIT_TAG        main # or any chosen commit
)

FetchContent_MakeAvailable(acync)

target_link_libraries(<your cmake project> PRIVATE acync)
```

#### Manual compilation
Dependencies:
 - cmake >= 3.23
 - cc with C11 support
 - pthreads

The compilation process can be done as such:

```
git clone -b main https://gitlab.com/TheMutta/acync.git
cd acync
mkdir build
cd build
cmake ..
make -j$(nproc)
```

In the build directory you can find the `libacync.so` file.
