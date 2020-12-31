project(AuroraEngine C CXX)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE PROJECT_SOURCE Source/*.hpp Source/*.cpp Source/*.h Source/*.c)

if(MINGW)
    add_compile_options(-mthreads -lpthread -msse4 -lstdc++fs)
    set(WIN32 ON)
    set(CMAKE_SYSTEM_NAME Windows)

    if(VL_TOOLS AND VL_TOOLS_CONSOLE AND CMAKE_BUILD_TYPE EQUAL "DEBUG")
        add_compile_options("-mconsole") # Open console window in addition to GLFW window
    else()
        add_compile_options("-mwindows") # Only GLFW window
    endif()
endif()

add_library(AuroraEngine STATIC ${PROJECT_SOURCE})

target_include_directories(AuroraEngine PUBLIC Source/)

if(WIN32)
    target_link_libraries(AuroraEngine PUBLIC -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
endif()

target_link_libraries(AuroraEngine PUBLIC AuroraCore glfw glm::glm)

########################################################################################
target_compile_options(AuroraEngine PUBLIC -DUNICODE -DENGINE_DLL)
target_include_directories(AuroraEngine PUBLIC "././External/DiligentEngine/DiligentCore")
target_include_directories(AuroraEngine PUBLIC "././External/DiligentEngine/DiligentTools/NativeApp")

target_link_libraries(AuroraEngine
        PUBLIC
        Diligent-Common
        Diligent-GraphicsTools
        Diligent-NativeAppBase
        Diligent-BuildSettings
        Diligent-GraphicsEngineOpenGL-shared
        Diligent-GraphicsEngineVk-shared
        )
install_tools_lib(AuroraEngine)
copy_required_dlls(AuroraEngine)
########################################################################################