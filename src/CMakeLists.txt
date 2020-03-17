add_executable(dht22 main.cpp)

target_link_libraries(dht22 PRIVATE dht -static)
set_property(TARGET dht22 PROPERTY CXX_STANDARD 20)

install(TARGETS dht22
        RUNTIME
          DESTINATION bin)
