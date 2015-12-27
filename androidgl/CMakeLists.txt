cmake_minimum_required(VERSION 3.0)

project(scene)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -llog -landroid -lGLESv2 -lz")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/../app/src/main/jniLibs/${ARCH}")

include_directories(libs)
add_subdirectory(libs/android)
add_subdirectory(libs/gl)
add_subdirectory(libs/gles2)

add_library(scene SHARED scene.cpp)
target_link_libraries(scene gl gles2 androidgl)

