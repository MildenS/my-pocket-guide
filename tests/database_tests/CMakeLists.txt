add_executable(database_tests
    database_tests.cpp
)

target_include_directories(database_tests PRIVATE ${DATABASE_INCLUDE_DIRS})

target_link_libraries(database_tests
    PRIVATE
        gtest
        gtest_main
        ${MPG_DATABASE_LIBRARY}
)

include(GoogleTest)
#gtest_discover_tests(database_tests)
