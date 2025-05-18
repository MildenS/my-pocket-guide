cmake_minimum_required(VERSION 3.16)
project(MyPocketGuideDatabase)
set(MPG_DATABASE_LIBRARY mpgDatabaseLib CACHE INTERNAL "Database library name")

find_package(OpenCV REQUIRED)

add_library(${MPG_DATABASE_LIBRARY} src/database_module/database.cpp)

set(CASSANDRA_STATIC_LIB
    ${CMAKE_SOURCE_DIR}/build/thirdparty/libcassandra_static.a
)

set(DATABASE_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/database/include
    ${CMAKE_SOURCE_DIR}/thirdparty/cpp-driver/include
    ${CMAKE_SOURCE_DIR}/utils
    ${OpenCV_INCLUDE_DIRS} CACHE PATH "Include directories for database"
)
message(STATUS "DATABASE_INCLUDE_DIRS: ${DATABASE_INCLUDE_DIRS}")

target_include_directories(${MPG_DATABASE_LIBRARY} PUBLIC ${DATABASE_INCLUDE_DIRS})

target_link_libraries(${MPG_DATABASE_LIBRARY} PUBLIC ${OpenCV_LIBS})
target_link_libraries(${MPG_DATABASE_LIBRARY} PUBLIC cassandra_static -ldl)
target_link_libraries(${MPG_DATABASE_LIBRARY} PUBLIC utils)


set_target_properties(${MPG_DATABASE_LIBRARY} PROPERTIES
    LINK_SEARCH_START_STATIC TRUE
    LINK_SEARCH_END_STATIC TRUE
)
#add_executable(${PROJECT_NAME} src/main.cpp)

#target_link_libraries(${PROJECT_NAME} ${MPG_DATABASE_LIBRARY})
