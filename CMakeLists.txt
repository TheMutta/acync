cmake_minimum_required(VERSION 3.23)

project(Acync VERSION 0.1)
find_package(Threads REQUIRED)

# library files
add_library(acync SHARED
	src/async.c
)

# setup the links and includes
target_link_libraries(acync PRIVATE Threads::Threads)
target_include_directories(acync PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

# build test only in repo
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	add_executable(acync_test
	    test/main.c
	)
	target_link_libraries(acync_test PRIVATE acync Threads::Threads)
endif()
