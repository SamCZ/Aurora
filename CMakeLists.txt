project(Aurora C CXX)
set(CMAKE_CXX_STANDARD 20)

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

if(WIN32)
    target_link_libraries(Aurora PRIVATE opengl32 gdi32)
endif()

target_link_libraries(Aurora PUBLIC glad glfw glm nlohmann_json EnTT TracyClient tinygltf ImGui TracyClient ndNewton glslang)