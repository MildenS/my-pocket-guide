
find_package(Threads REQUIRED) #need for spdlog::init_thread_pool
add_library(utils INTERFACE)

target_include_directories(utils
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/thirdparty/spdlog/include
)

target_link_libraries(utils
    INTERFACE
        Threads::Threads
)