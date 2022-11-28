cmake_minimum_required(VERSION 3.19)
project(Aurora C CXX)
set(CMAKE_CXX_STANDARD 20)

option(AU_GPU_PROFILE "GPU profiling" ON)
option(AU_CPU_PROFILE "CPU profiling" OFF)
option(AU_FMOD_SOUND "Enable FMOD sound system" OFF)
option(AU_IN_PROJECT_ASSETS "Adds Aurora search paths from source code" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTING "Build tests" ON)

set(FMOD_API_DIR "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api" CACHE STRING "FMOD api folder")

set(RUNTIME_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for dlls and binaries")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for binaries")
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for dlls")

IF(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for dlls and binaries")
endif()

#if(MSVC)
#    # Force to always compile with W4
#    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
#        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#    else()
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
#    endif()
#elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
#    # Update if necessary
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long")
#endif()

#SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for binaries")
#SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory for dlls")

# We do not support shared libs on linux system
if(UNIX)
    set(BUILD_SHARED_LIBS OFF)
endif()

set(AU_ROOT_SOURCE_DIR ${PROJECT_SOURCE_DIR})

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory(lib)
add_subdirectory(Aurora)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()