cmake_minimum_required(VERSION 3.16.5)
project(Aurora C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
cmake_policy(SET CMP0077 NEW) # set() overrides option()

add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>") # Define DEBUG in Debug builds

if(MINGW)
    message(NOTICE "MinGW detected, setting compilation to Windows")
    set(WIN32 ON)
endif()

add_subdirectory(External)

file(GLOB_RECURSE PROJECT_SOURCE Source/*.hpp Source/*.cpp Source/*.h Source/*.c)

add_library(Aurora STATIC ${PROJECT_SOURCE})
target_include_directories(Aurora PUBLIC Source/)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(Aurora PRIVATE -static-libgcc -static-libstdc++)
endif()
########################################################################################
target_compile_options(Aurora PRIVATE -DUNICODE -DENGINE_DLL)
target_include_directories(Aurora PRIVATE "External/DiligentEngine/DiligentCore")

target_link_libraries(Aurora
        PRIVATE
        Diligent-GraphicsEngineOpenGL-shared
        Diligent-GraphicsEngineVk-shared
        )
copy_required_dlls(Aurora)
########################################################################################
if(BUILD_TESTING)
    include(CTest)
    enable_testing()

    add_subdirectory(tests)
endif()