add_custom_command(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/Kbuild"
    COMMAND  ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/Kbuild"
            "${CMAKE_CURRENT_BINARY_DIR}/Kbuild"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Kbuild")

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/dht.ko"
    COMMAND make -C "/lib/modules/${CMAKE_SYSTEM_VERSION}/build"
                    M="${CMAKE_CURRENT_BINARY_DIR}"
                    src="${CMAKE_CURRENT_SOURCE_DIR}"
                    modules
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/dht.c" "${CMAKE_CURRENT_BINARY_DIR}/Kbuild"
    USES_TERMINAL)

add_custom_target(kmod DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/dht.ko")
