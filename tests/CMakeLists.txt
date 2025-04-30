if(NOT MPG_BUILD_TESTS)
    return()
endif()

add_subdirectory(thirdparty/googletest)
add_subdirectory(database_tests)