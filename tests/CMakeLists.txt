add_executable(tests main.cpp device_tests.cpp iterator_tests.cpp gpio_tests.cpp)
target_link_libraries(tests doctest dht)
set_property(TARGET tests PROPERTY CXX_STANDARD 17)
# Don't run cppcheck since we include doctest
set_property(TARGET tests PROPERTY CXX_CPPCHECK)

include(CTest)
doctest_discover_tests(tests)
