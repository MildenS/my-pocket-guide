add_executable(core_tests
    core_tests.cpp
)

target_include_directories(core_tests PRIVATE ${CORE_INCLUDE_DIRS})

target_link_libraries(core_tests
    PRIVATE
        gtest
        gtest_main
        ${MPG_CORE_LIBRARY}
        utils
)

include(GoogleTest)
#gtest_discover_tests(core_tests)
