add_executable(gpio-event-mon gpio_event_mon.cpp)
target_link_libraries(gpio-event-mon dht -static)

install(TARGETS gpio-event-mon
        RUNTIME
          DESTINATION bin)
