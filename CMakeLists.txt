cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x")
set(Boost_USE_STATIC_LIBS ON)
set(HALIDE_DIR "/Users/mzhang94/Halide")
project(Lightroom)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(perf)
