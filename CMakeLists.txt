cmake_minimum_required(VERSION 3.19)
project(Aurora C CXX)
set(CMAKE_CXX_STANDARD 20)

option(AU_GPU_PROFILE "GPU profiling" ON)
option(AU_CPU_PROFILE "CPU profiling" OFF)
option(AU_IN_PROJECT_ASSETS "Adds Aurora search paths from source code" ON)

SET(RUNTIME_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for dlls and binaries")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for binaries")
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for dlls")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory(lib)

file(GLOB_RECURSE PROJECT_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)

if(NOT BUILD_SHARED_LIBS)
    add_library(Aurora STATIC ${PROJECT_SOURCE})
    target_compile_definitions(Aurora PUBLIC AU_SHARED=0)
else()
    add_library(Aurora SHARED ${PROJECT_SOURCE})
    target_compile_definitions(Aurora PUBLIC AU_SHARED=1)
    target_compile_definitions(Aurora PRIVATE AU_EXPORT=1)
endif()

if(AU_IN_PROJECT_ASSETS)
    target_compile_definitions(Aurora PUBLIC AU_IN_PROJECT_ASSETS=1)
else()
    target_compile_definitions(Aurora PUBLIC AU_IN_PROJECT_ASSETS=0)
endif()

target_include_directories(Aurora PUBLIC src)
target_include_directories(Aurora PUBLIC Assets)

target_compile_definitions(Aurora PUBLIC GLFW_INCLUDE_NONE=1)
target_compile_definitions(Aurora PUBLIC GLFW_ENABLED=1)
#target_compile_definitions(Aurora PUBLIC RMLUI_STATIC_LIB=1)
target_compile_definitions(Aurora PUBLIC AURORA_OPENGL=1)
target_compile_definitions(Aurora PUBLIC SHADER_ENGINE_SIDE=1)
target_compile_definitions(Aurora PUBLIC AURORA_PROJECT_DIR="${PROJECT_SOURCE_DIR}")

target_compile_definitions(Aurora PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_compile_definitions(Aurora PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(Aurora PUBLIC GLM_EXT_INCLUDED)

if(AU_GPU_PROFILE)
    target_compile_definitions(Aurora PUBLIC AU_GPU_PROFILE=1)
    target_compile_definitions(Aurora PUBLIC OPENGL_ERROR_CHECKING=1)
else()
    target_compile_definitions(Aurora PUBLIC AU_GPU_PROFILE=0)
    target_compile_definitions(Aurora PUBLIC OPENGL_ERROR_CHECKING=0)
endif()

if(AU_CPU_PROFILE)
    target_compile_definitions(Aurora PUBLIC AU_TRACY_ENABLED=1)
else()
    target_compile_definitions(Aurora PUBLIC AU_TRACY_ENABLED=0)
endif()

target_compile_definitions(Aurora PUBLIC GLSLANG_COMPILER=1)

if(WIN32)
    target_link_libraries(Aurora PRIVATE opengl32 gdi32)
endif()

target_link_libraries(Aurora PUBLIC glad glfw glm nlohmann_json TracyClient ImGui TracyClient assimp stb_utils)
target_link_libraries(Aurora PUBLIC RmlCore RmlDebugger)

add_subdirectory(examples)