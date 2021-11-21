project(Aurora C CXX)
set(CMAKE_CXX_STANDARD 20)

option(AU_GPU_PROFILE "GPU profiling" ON)
option(AU_CPU_PROFILE "CPU profiling" ON)

add_subdirectory(lib)

file(GLOB_RECURSE PROJECT_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)
add_library(Aurora STATIC ${PROJECT_SOURCE})

target_include_directories(Aurora PUBLIC src)
target_include_directories(Aurora PUBLIC Assets)

target_compile_definitions(Aurora PUBLIC GLFW_INCLUDE_NONE=1)
target_compile_definitions(Aurora PUBLIC GLFW_ENABLED=1)
target_compile_definitions(Aurora PUBLIC RMLUI_STATIC_LIB=1)
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

target_link_libraries(Aurora PUBLIC glad glfw glm nlohmann_json EnTT TracyClient tinygltf ImGui TracyClient ndNewton glslang)
target_link_libraries(Aurora PUBLIC RmlCore RmlDebugger)