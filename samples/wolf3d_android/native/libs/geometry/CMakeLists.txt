cmake_minimum_required(VERSION 3.0)

project(geometry)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

set(srcs 
	utility.cpp )

add_library(geometry STATIC ${srcs})

