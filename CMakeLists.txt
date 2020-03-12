# Author: Emil Gedda

cmake_minimum_required(VERSION 3.12)
project(dht CXX)

find_program(CPPCHECK NAMES cppcheck)
if (CPPCHECK)
    message(STATUS "Found cppcheck: ${CPPCHECK}")
    list(
        APPEND CMAKE_CXX_CPPCHECK
            "${CPPCHECK}"
            "--enable=all"
            "--inline-suppr"
            "--suppress=unusedFunction"
            "--suppress=constParameter"
            "--suppress=unmatchedSuppression"
            "--suppress=missingIncludeSystem"
            "--error-exitcode=1"
    )
endif()

find_program(IWYU NAMES "include-what-you-use")
if (IWYU)
    message(STATUS "Found include-what-you-use: ${IWYU}")
    list(APPEND CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU}")
endif()

add_subdirectory("lib")
add_subdirectory("src")
add_subdirectory("doc")
