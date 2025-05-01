cmake_minimum_required(VERSION 3.16)
project(MyPocketGuideDatabase)
set(MPG_DATABASE_LIBRARY mpgDatabaseLib CACHE INTERNAL "Database library name")

add_subdirectory(thirdparty/cpp-driver)
find_package(OpenCV REQUIRED)

add_library(${MPG_DATABASE_LIBRARY} src/database_module/database.cpp)

set(DATABASE_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/database/include
    ${CMAKE_SOURCE_DIR}/database/thirdparty/cpp-driver/include
    ${OpenCV_INCLUDE_DIRS} CACHE PATH "Include directories for database"
)

target_include_directories(${MPG_DATABASE_LIBRARY} PUBLIC ${DATABASE_INCLUDE_DIRS})

target_link_libraries(${MPG_DATABASE_LIBRARY} ${OpenCV_LIBS})
target_link_libraries(${MPG_DATABASE_LIBRARY} cassandra)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} ${MPG_DATABASE_LIBRARY})
