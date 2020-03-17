add_library(dht device.cpp iterator.cpp gpio.cpp)

target_include_directories(dht PUBLIC "${PROJECT_SOURCE_DIR}/inc")
set_property(TARGET dht PROPERTY CXX_STANDARD 17)
target_compile_features(dht PUBLIC cxx_std_17)

install(TARGETS dht
        LIBRARY
          DESTINATION lib/dht
        ARCHIVE
          DESTINATION lib/dht
        PUBLIC_HEADER
          DESTINATION include)

install(FILES "${PROJECT_SOURCE_DIR}/LICENSE"
        DESTINATION share/licenses/libdht)

install(FILES "${PROJECT_SOURCE_DIR}/README.md"
        DESTINATION share/doc/libdht)
