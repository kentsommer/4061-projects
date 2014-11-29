cmake_minimum_required(VERSION 2.8.4)
project(project4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    makefile
    server.c
    util.h)

add_executable(project4 ${SOURCE_FILES})