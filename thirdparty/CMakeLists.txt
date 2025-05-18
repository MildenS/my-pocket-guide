
set(CASS_BUILD_STATIC ON)
set(CASS_BUILD_SHARED OFF)

set(CASS_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(CASS_BUILD_STATIC ON CACHE BOOL "" FORCE)

add_subdirectory(cpp-driver)
add_subdirectory(googletest)