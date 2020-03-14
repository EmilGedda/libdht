file(GLOB_RECURSE CMAKE_CFG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")
foreach(CFG_FILE ${CMAKE_CFG_FILES})
    include("${CFG_FILE}")
endforeach()
