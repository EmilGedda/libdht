# Emil Gedda 2020
cmake_minimum_required(VERSION 3.12)
project(libdht CXX)

include("cmake/CMakeLists.txt") # Don't create a new scope
add_subdirectory("mod")
add_subdirectory("lib")
add_subdirectory("src")
add_subdirectory("doc")
add_subdirectory("tests")
add_subdirectory("tools")

enable_testing()
