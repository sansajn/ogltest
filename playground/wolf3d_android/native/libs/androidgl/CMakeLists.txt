cmake_minimum_required(VERSION 3.0)

project(androidgl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -llog -landroid
-lGLESv2")

set(srcs 
	android_window.cpp)

add_library(androidgl STATIC ${srcs})

