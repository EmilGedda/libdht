# Author: Emil Gedda
cmake_minimum_required(VERSION 3.12)
project(dht CXX)

find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
if (CMAKE_CXX_CPPCHECK)
    message(STATUS "Found cppcheck: ${CMAKE_CXX_CPPCHECK}")
    list(
        APPEND CMAKE_CXX_CPPCHECK
            "--enable=all"
            "--inline-suppr"
            "--suppress=unusedFunction"
            "--suppress=constParameter"
            "--suppress=unmatchedSuppression"
            "--suppress=missingIncludeSystem"
            "--error-exitcode=1"
    )
endif()

find_program(CMAKE_CXX_INCLUDE_WHAT_YOU_USE NAMES "include-what-you-use")
if (CMAKE_CXX_CPPCHECK)
    message(STATUS "Found include-what-you-use: ${CMAKE_CXX_INCLUDE_WHAT_YOU_USE}")
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/lib")
add_subdirectory("${PROJECT_SOURCE_DIR}/src")
