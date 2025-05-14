if(NOT MPG_BUILD_TESTS)
    return()
endif()

add_subdirectory(database_tests)
add_subdirectory(core_tests)