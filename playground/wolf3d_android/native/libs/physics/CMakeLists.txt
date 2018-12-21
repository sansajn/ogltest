cmake_minimum_required(VERSION 3.0)

project(physics)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

include_directories(${SYSROOT}/usr/include/bullet)

set(srcs 
	debug_draw_gles2.cpp
	physics.cpp)

add_library(physics STATIC ${srcs})

