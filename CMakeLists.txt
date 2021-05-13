project(Aurora C CXX)
cmake_policy(SET CMP0077 NEW) # set() overrides option()

file(GLOB_RECURSE PROJECT_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)

add_library(Aurora STATIC ${PROJECT_SOURCE} src/Aurora/Profiler/Profiler.cpp src/Aurora/Profiler/Profiler.hpp)

target_include_directories(Aurora PUBLIC src)
target_link_libraries(Aurora PUBLIC glfw glm glad assimp nlohmann_json)

# Link Dilligent
target_compile_options(Aurora PUBLIC -DUNICODE -DENGINE_DLL)

target_link_libraries(Aurora
        PUBLIC
        Diligent-Common
        Diligent-GraphicsTools
        Diligent-NativeAppBase
        Diligent-BuildSettings
        Diligent-TextureLoader
        Diligent-Imgui
        Diligent-GraphicsAccessories
)

if(WIN32)
    target_link_libraries(Aurora PUBLIC FMod_SDK)
    add_compile_definitions(FMOD_SUPPORTED=1)
endif()

if(ES_PRODUCTION)
    target_link_libraries(Aurora PUBLIC Diligent-GraphicsEngineVk-static)
else()
    target_link_libraries(Aurora PUBLIC Diligent-GraphicsEngineVk-shared)
endif()

if(UNIX)
    target_link_libraries(Aurora PUBLIC X11-xcb)
endif()