cmake_minimum_required(VERSION 3.0)

project(gles2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lGLESv2")

set(srcs 
	default_shader_gles2.cpp 
	mesh_gles2.cpp
	program_gles2.cpp
	touch_joystick_gles2.cpp)

add_library(gles2 STATIC ${srcs})

