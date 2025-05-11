cmake_minimum_required(VERSION 3.16)
project(MyPocketGuideDatabase)
set(MPG_DATABASE_LIBRARY mpgDatabaseLib CACHE INTERNAL "Database library name")

find_package(OpenCV REQUIRED)

add_library(${MPG_DATABASE_LIBRARY} src/database_module/database.cpp)


set(DATABASE_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/database/include
    ${CMAKE_SOURCE_DIR}/thirdparty/cpp-driver/include
    ${CMAKE_SOURCE_DIR}/utils
    ${OpenCV_INCLUDE_DIRS} CACHE PATH "Include directories for database"
)
message(STATUS "DATABASE_INCLUDE_DIRS: ${DATABASE_INCLUDE_DIRS}")

target_include_directories(${MPG_DATABASE_LIBRARY} PUBLIC ${DATABASE_INCLUDE_DIRS})

target_link_libraries(${MPG_DATABASE_LIBRARY} ${OpenCV_LIBS})
target_link_libraries(${MPG_DATABASE_LIBRARY} cassandra)
target_link_libraries(${MPG_DATABASE_LIBRARY} utils)

#add_executable(${PROJECT_NAME} src/main.cpp)

#target_link_libraries(${PROJECT_NAME} ${MPG_DATABASE_LIBRARY})
