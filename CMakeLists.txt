# Author: Emil Gedda
cmake_minimum_required(VERSION 3.12)
project(dht CXX)


add_library(dht src/dht.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++2a")

add_executable(dht22 src/main.cpp)
target_link_libraries(dht22 PRIVATE dht)
