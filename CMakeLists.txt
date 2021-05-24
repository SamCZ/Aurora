project(Aurora C CXX)
cmake_policy(SET CMP0077 NEW) # set() overrides option()

file(GLOB_RECURSE PROJECT_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)

add_library(Aurora STATIC ${PROJECT_SOURCE})

target_include_directories(Aurora PUBLIC src)

target_compile_definitions(Aurora PUBLIC GLFW_INCLUDE_NONE=1)

target_link_libraries(Aurora PUBLIC glfw glm assimp nlohmann_json)
target_link_libraries(Aurora PUBLIC glad opengl32 gdi32)

if(WIN32)
    target_link_libraries(Aurora PUBLIC FMod_SDK)
    add_compile_definitions(FMOD_SUPPORTED=1)
endif()

if(UNIX)
    target_link_libraries(Aurora PUBLIC X11-xcb)
endif()