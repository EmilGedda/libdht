include(ExternalProject)

find_package(Git REQUIRED)

ExternalProject_Add(
    projdoctest
    PREFIX ${CMAKE_BINARY_DIR}/doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest.git
    GIT_TAG "2.3.7"
    TIMEOUT 30
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    GIT_SHALLOW ON
    LOG_OUTPUT_ON_FAILURE ON
    LOG_MERGED_STDOUTERR ON
)

# Expose required variable (DOCTEST_INCLUDE_DIR) to parent scope
ExternalProject_Get_Property(projdoctest source_dir)
add_library(doctest INTERFACE)

add_dependencies(doctest projdoctest)
target_compile_features(doctest INTERFACE cxx_std_17)
target_include_directories(doctest INTERFACE ${source_dir})

# Include `doctest_discover_tests` function for CTest integration
if(NOT EXISTS "${CMAKE_BINARY_DIR}/doctest/doctest.cmake")
  message(STATUS "Downloading doctest CMake integration script")
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/doctest")
  file(DOWNLOAD "https://raw.githubusercontent.com/onqtam/doctest/master/scripts/cmake/doctest.cmake" "${CMAKE_BINARY_DIR}/doctest/doctest.cmake")
  file(DOWNLOAD "https://raw.githubusercontent.com/onqtam/doctest/master/scripts/cmake/doctestAddTests.cmake" "${CMAKE_BINARY_DIR}/doctest/doctestAddTests.cmake")
else()
  message(STATUS "Using already downloaded integration script")
endif()

include("${CMAKE_BINARY_DIR}/doctest/doctest.cmake")
