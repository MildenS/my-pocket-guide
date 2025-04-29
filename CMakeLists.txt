cmake_minimum_required(VERSION 3.16)
project(MyPocketGuide)
set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wunused-result -Wunused)
endif()

add_subdirectory(database)
add_subdirectory(server)