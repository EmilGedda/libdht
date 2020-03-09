add_executable(dht22 main.cpp)

target_link_libraries(dht22 PRIVATE dht)
set_property(TARGET dht22 PROPERTY CXX_STANDARD 20)
