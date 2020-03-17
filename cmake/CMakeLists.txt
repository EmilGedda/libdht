file(GLOB_RECURSE CMAKE_CFG_FILES "${CMAKE_CURRENT_LIST_DIR}/*.cmake")
foreach(CFG_FILE ${CMAKE_CFG_FILES})
    include("${CFG_FILE}")
endforeach()
