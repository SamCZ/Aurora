cmake_minimum_required(VERSION 3.16.5)
project(Aurora)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
cmake_policy(SET CMP0077 NEW) # set() overrides option()

add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>") # Define DEBUG in Debug builds

if(MINGW)
    message(NOTICE "MinGW detected, setting compilation to Windows")
    set(WIN32 ON)
endif()

add_subdirectory(External)
add_subdirectory(Source)

if(BUILD_TESTING)
    include(CTest)
    enable_testing()

    add_subdirectory(tests)
endif()