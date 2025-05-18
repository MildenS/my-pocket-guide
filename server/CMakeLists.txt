cmake_minimum_required(VERSION 3.16)
project(MyPocketGuideServer)

set(MPG_SERVER_LIBRARY mpgServerLib CACHE INTERNAL "Server library name")


add_library(${MPG_SERVER_LIBRARY} src/server/server.cpp)


set(SERVER_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/server/include
    ${CMAKE_SOURCE_DIR}/utils 
    ${CORE_INCLUDE_DIRS} CACHE PATH "Include directories for core"
)
message(STATUS "SERVER_INCLUDE_DIRS: ${SERVER_INCLUDE_DIRS}")

find_package(wfrest REQUIRED)

target_include_directories(${MPG_SERVER_LIBRARY} PUBLIC ${SERVER_INCLUDE_DIRS})

target_link_libraries(${MPG_SERVER_LIBRARY} PRIVATE ${MPG_CORE_LIBRARY})
target_link_libraries(${MPG_SERVER_LIBRARY} PUBLIC utils wfrest)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${MPG_SERVER_LIBRARY})
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${MPG_SERVER_LIBRARY}
    ${MPG_CORE_LIBRARY}
    utils
    wfrest
    cassandra_static
)