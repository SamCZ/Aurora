project(AuroraCore C CXX)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE PROJECT_SOURCE Source/*.hpp Source/*.cpp Source/*.h Source/*.c)

if(MINGW)
    #set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
    add_compile_options(-mthreads -lpthread -msse4 -lstdc++fs)
    set(WIN32 ON)
    set(CMAKE_SYSTEM_NAME Windows)

    if(VL_TOOLS AND VL_TOOLS_CONSOLE AND CMAKE_BUILD_TYPE EQUAL "DEBUG")
        add_compile_options("-mconsole") # Open console window in addition to GLFW window
    else()
        add_compile_options("-mwindows") # Only GLFW window
    endif()
endif()

add_library(AuroraCore STATIC ${PROJECT_SOURCE})

target_include_directories(AuroraCore PUBLIC Source/)

if(WIN32)
    target_link_libraries(AuroraCore PUBLIC -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
endif()

target_link_libraries(AuroraCore PUBLIC glfw glm::glm)

target_link_libraries(AuroraCore PUBLIC Diligent-BuildSettings Diligent-Common)