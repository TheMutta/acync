cmake_minimum_required(VERSION 3.23)

project(Acync VERSION 0.1)

find_package(Threads REQUIRED)

add_library(acync SHARED
	src/async.c
)

target_link_libraries(acync PRIVATE Threads::Threads)
target_include_directories(acync PUBLIC include)

add_executable(acync_test
    test/main.c
)
target_link_libraries(acync_test PRIVATE acync Threads::Threads)
