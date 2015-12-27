cmake_minimum_required(VERSION 3.0)

project(gl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

set(srcs 
	camera.cpp 
	window.cpp )

add_library(gl STATIC ${srcs})

