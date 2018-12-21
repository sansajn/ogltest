cmake_minimum_required(VERSION 3.0)

project(al)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lOpenSLES")

set(srcs 
	audio.cpp
	blob_wave.cpp
	waveform.cpp
	vorbis.cpp
)

add_library(al STATIC ${srcs})
target_link_libraries(al openal.a vorbisfile.a vorbis.a ogg.a)
