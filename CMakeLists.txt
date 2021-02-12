cmake_minimum_required(VERSION 3.16.5)
project(Aurora C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
cmake_policy(SET CMP0077 NEW) # set() overrides option()

add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>") # Define DEBUG in Debug builds

if(MINGW)
    message(NOTICE "MinGW detected, setting compilation to Windows")
    add_compile_options(-mthreads -lpthread -msse4 -lstdc++fs)
    set(WIN32 ON)
    set(CMAKE_SYSTEM_NAME Windows)

    if(VL_TOOLS AND VL_TOOLS_CONSOLE AND CMAKE_BUILD_TYPE EQUAL "DEBUG")
        add_compile_options("-mconsole") # Open console window in addition to GLFW window
    else()
        add_compile_options("-mwindows") # Only GLFW window
    endif()
endif()

add_subdirectory(lib)

####################################################

file(GLOB_RECURSE PROJECT_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)
add_library(AuroraEngine STATIC ${PROJECT_SOURCE})
target_include_directories(AuroraEngine PUBLIC src/)

target_compile_definitions(AuroraEngine PUBLIC AURORA_PROJECT_SOURCE_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(AuroraEngine PUBLIC AURORA_PROJECT_BINARY_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}")

if(MINGW)
    target_link_libraries(AuroraEngine PUBLIC -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
endif()

target_link_libraries(AuroraEngine PUBLIC glfw Vulkan::Vulkan glm::glm)

####################################################

if(BUILD_TESTING)
    #include(CTest)
    enable_testing()

    add_subdirectory(tests)
endif()