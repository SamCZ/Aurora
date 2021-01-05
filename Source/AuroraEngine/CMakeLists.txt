project(AuroraEngine C CXX)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE PROJECT_SOURCE Source/*.hpp Source/*.cpp Source/*.h Source/*.c)

add_library(AuroraEngine STATIC ${PROJECT_SOURCE})

target_include_directories(AuroraEngine PUBLIC Source/)

target_link_libraries(AuroraEngine PUBLIC -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)

target_link_libraries(AuroraEngine PUBLIC AuroraCore glfw glm::glm)

########################################################################################
target_compile_options(AuroraEngine PUBLIC -DUNICODE -DENGINE_DLL)
target_include_directories(AuroraEngine PUBLIC ./././External/DiligentEngine/DiligentCore/)
target_include_directories(AuroraEngine PUBLIC ./././External/DiligentEngine/DiligentTools/NativeApp/)

target_link_libraries(AuroraEngine
        PUBLIC
        Diligent-Common
        Diligent-GraphicsTools
        Diligent-NativeAppBase
        Diligent-BuildSettings
        Diligent-TextureLoader
        Diligent-GraphicsEngineOpenGL-shared
        Diligent-GraphicsEngineVk-shared
        )
install_tools_lib(AuroraEngine)
copy_required_dlls(AuroraEngine)
########################################################################################