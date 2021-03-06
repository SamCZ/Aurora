cmake_minimum_required(VERSION 3.19)
project(Aurora C CXX)
set(CMAKE_CXX_STANDARD 20)

set(FMOD_API_DIR "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api" CACHE STRING "Fmod api folder")
set(BUILD_EXAMPLES OFF CACHE STRING "Build examples")

set(AURORA_DLL_TO_COPY "" CACHE INTERNAL "AURORA_DLL_TO_COPY")

if(WIN32 AND NOT PLATFORM_ANDROID)
    set(FMOD_SUPPORTED ON)
endif()

add_subdirectory(lib)

if(FMod_NotFound)
    set(FMOD_SUPPORTED OFF)
else()
    message("FMod successfully loaded.")
endif()

file(GLOB_RECURSE PROJECT_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)
add_library(Aurora STATIC ${PROJECT_SOURCE})

target_compile_definitions(Aurora PUBLIC GLFW_INCLUDE_NONE=1)
target_compile_definitions(Aurora PUBLIC GLFW_ENABLED=1)

target_include_directories(Aurora PUBLIC src)

if(FMOD_SUPPORTED)
    target_link_libraries(Aurora PUBLIC FMod)
    target_compile_definitions(Aurora PUBLIC FMOD_SUPPORTED=1)
endif()

target_link_libraries(Aurora PUBLIC glfw glm nlohmann_json assimp)
target_link_libraries(Aurora PUBLIC glad opengl32 gdi32)
target_link_libraries(Aurora PUBLIC ImGui)

function(link_aurora TARGET_NAME)
    target_link_libraries(${TARGET_NAME} PUBLIC Aurora)
    target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)

    foreach(DLL ${AURORA_DLL_TO_COPY})
        #configure_file(${DLL} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy
                ${DLL}
                ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Copying dll ${DLL} to output directory")
    endforeach(DLL)
endfunction()

# Build examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()