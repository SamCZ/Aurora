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
target_compile_definitions(Aurora PUBLIC RMLUI_STATIC_LIB=1)

target_include_directories(Aurora PUBLIC src)

if(FMOD_SUPPORTED)
    # TODO: Fix FMod
    if(NOT MSVC)
        #target_link_libraries(Aurora PUBLIC FMod)
        #target_compile_definitions(Aurora PUBLIC FMOD_SUPPORTED=1)
    endif()
endif()

target_compile_definitions(Aurora PUBLIC NOMINMAX)

target_link_libraries(Aurora PUBLIC glfw glm nlohmann_json assimp)
target_link_libraries(Aurora PUBLIC glad)
target_link_libraries(Aurora PUBLIC ImGui)
target_link_libraries(Aurora PUBLIC RmlCore RmlDebugger)

if(WIN32)
    target_link_libraries(Aurora PUBLIC opengl32 gdi32)
endif()

target_link_libraries(Aurora PUBLIC ndNewton)
target_link_libraries(Aurora PUBLIC TracyClient)

foreach(INCLUDE_DIR ${AURORA_ADD_INCLUDE_DIRS})
    target_include_directories(Aurora PUBLIC ${INCLUDE_DIR})
endforeach(INCLUDE_DIR)

function(link_aurora TARGET_NAME)
    target_link_libraries(${TARGET_NAME} PUBLIC Aurora)
    target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)

    if(WIN32)
        foreach(DLL ${AURORA_DLL_TO_COPY})
            #configure_file(${DLL} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E copy
                    ${DLL}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    COMMENT "Copying dll ${DLL} to output directory")
        endforeach(DLL)
    endif(WIN32)
endfunction()

# Build examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()