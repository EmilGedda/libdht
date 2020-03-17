include(CTest)

find_package(Threads REQUIRED)

add_executable(gpio_test EXCLUDE_FROM_ALL gpio_tests.cpp)
target_link_libraries(gpio_test dht doctest Threads::Threads)
set_property(TARGET gpio_test PROPERTY CXX_STANDARD 17)

# Don't run cppcheck or IWYU on tests
set_property(TARGET gpio_test PROPERTY CXX_CPPCHECK)
doctest_discover_tests(gpio_test)
