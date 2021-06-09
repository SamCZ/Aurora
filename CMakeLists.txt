project(Aurora C CXX)
cmake_policy(SET CMP0077 NEW) # set() overrides option()

option(PLATFORM_ANDROID "Use android platform" OFF)

file(GLOB_RECURSE PROJECT_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)

add_library(Aurora STATIC ${PROJECT_SOURCE})

target_include_directories(Aurora PUBLIC src)

if(NOT PLATFORM_ANDROID)
    target_compile_definitions(Aurora PUBLIC GLFW_INCLUDE_NONE=1)
    target_compile_definitions(Aurora PUBLIC GLFW_ENABLED=1)
    target_link_libraries(Aurora PUBLIC glfw)
    target_link_libraries(Aurora PUBLIC glad opengl32 gdi32)
else()
    target_compile_definitions(Aurora PUBLIC PLATFORM_ANDROID=1)
    target_link_libraries(Aurora PUBLIC android EGL GLESv3 libc++)
endif()

target_link_libraries(Aurora PUBLIC glm assimp nlohmann_json)

if(WIN32 AND NOT PLATFORM_ANDROID)
    target_link_libraries(Aurora PUBLIC FMod_SDK)
    add_compile_definitions(FMOD_SUPPORTED=1)
endif()

if(UNIX AND NOT PLATFORM_ANDROID)
    target_link_libraries(Aurora PUBLIC X11-xcb)
endif()