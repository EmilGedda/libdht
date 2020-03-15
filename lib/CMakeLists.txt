add_library(dht device.cpp iterator.cpp gpio.cpp)

target_include_directories(dht PUBLIC "${PROJECT_SOURCE_DIR}/inc")
set_property(TARGET dht PROPERTY CXX_STANDARD 17)
target_compile_features(dht PUBLIC cxx_std_17)
