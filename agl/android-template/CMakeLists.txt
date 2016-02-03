cmake_minimum_required(VERSION 3.0)

project(scene)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -llog -landroid")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/android/libs/${ARCH}")

include_directories(libs)

add_subdirectory(libs/gl)
add_subdirectory(libs/androidgl)

set(srcs
	main.cpp
)

add_library(scene SHARED ${srcs})
target_link_libraries(scene androidgl gl)
