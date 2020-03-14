find_program(CPPCHECK NAMES cppcheck)
if (CPPCHECK)
    message(STATUS "Found cppcheck: ${CPPCHECK}")
    list(
        APPEND CMAKE_CXX_CPPCHECK
            "${CPPCHECK}"
            "--enable=all"
            "--inline-suppr"
            "--suppress=unusedFunction"
            "--suppress=constParameter"
            "--suppress=unmatchedSuppression"
            "--suppress=missingIncludeSystem"
            "--error-exitcode=1"
    )
endif()
