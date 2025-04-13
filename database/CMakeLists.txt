cmake_minimum_required(VERSION 3.16)
project(MyPocketGuideDatabase)
set(MPG_DATABASE_LIBRARY mpgDatabase)

set (CMAKE_CXX_STANDARD 17)

add_subdirectory(thirdparty/cpp-driver)
find_package(OpenCV REQUIRED)

add_library(${MPG_DATABASE_LIBRARY} src/database_module/database.cpp)

target_include_directories(${MPG_DATABASE_LIBRARY} PUBLIC ${OpenCV_INCLUDE_DIRS} cassandra)
target_include_directories(${MPG_DATABASE_LIBRARY} PUBLIC thirdparty/cpp-driver/include)

target_include_directories(${MPG_DATABASE_LIBRARY} PUBLIC thirdparty/cpp-driver/include)
target_link_libraries(${MPG_DATABASE_LIBRARY} cassandra)

target_include_directories(${MPG_DATABASE_LIBRARY} PUBLIC include/ )

add_executable(${PROJECT_NAME} src/main.cpp)

# #target_link_libraries(couchbase_cxx_client_static fmt::fmt)
 target_link_libraries(${PROJECT_NAME} ${MPG_DATABASE_LIBRARY})

# add_executable(TEMP src/main.cpp)

#target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} ${MPG_DATABASE_LIBRARY})

# target_include_directories(${PROJECT_NAME} PUBLIC thirdparty/PEGTL/include)
# target_include_directories(${PROJECT_NAME} PUBLIC thirdparty/json/include)