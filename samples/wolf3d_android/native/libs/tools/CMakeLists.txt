cmake_minimum_required(VERSION 3.0)

project(tools)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

set(srcs 
	boost/filesystem/directory_iterator.cpp
	boost/filesystem/operations.cpp
	boost/filesystem/path.cpp
)

add_library(tools STATIC ${srcs})
