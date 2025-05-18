cmake_minimum_required(VERSION 3.16)
project(MyPocketGuideCore)
set(MPG_CORE_LIBRARY mpgCoreLib CACHE INTERNAL "Core library name")


add_library(${MPG_CORE_LIBRARY} src/core_module/core.cpp)


set(CORE_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/utils 
    ${DATABASE_INCLUDE_DIRS} CACHE PATH "Include directories for core"
)
message(STATUS "CORE_INCLUDE_DIRS: ${CORE_INCLUDE_DIRS}")

target_include_directories(${MPG_CORE_LIBRARY} PUBLIC ${CORE_INCLUDE_DIRS})

target_link_libraries(${MPG_CORE_LIBRARY} PUBLIC ${MPG_DATABASE_LIBRARY})
target_link_libraries(${MPG_CORE_LIBRARY} PRIVATE utils)

#add_executable(${PROJECT_NAME} src/main.cpp)

#target_link_libraries(${PROJECT_NAME} ${MPG_CORE_LIBRARY})
