cmake_policy(SET CMP0077 NEW) # set() overrides option()

#------------------------------
# GLFW
# Desktop Window Library
#------------------------------
if(NOT TARGET glfw AND NOT PLATFORM_ANDROID)
    set(GLFW_INSTALL OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_DOCS OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(GLFW_USE_HYBRID_HPG ON)
    add_subdirectory(glfw)
    set_property(TARGET glfw PROPERTY FOLDER "GameLib")
endif()

#--------------------------------
# nlohmann JSON
# JSON for Modern C++
#--------------------------------
if(NOT TARGET nlohmann_json)
    set(JSON_BuildTests OFF)
    set(JSON_Install OFF)
    add_subdirectory(nlohmann_json)
    set_property(TARGET nlohmann_json PROPERTY FOLDER "GameLib")
endif()

#------------------------------
# GLFW
# Game mathematics library
#------------------------------
if(NOT TARGET glm)
    add_subdirectory(glm)
    set_property(TARGET glm PROPERTY FOLDER "GameLib")
endif()

#------------------------------
# GLAD
# OpenGL
#------------------------------
if(NOT TARGET glad AND NOT PLATFORM_ANDROID)
    add_subdirectory(glad)
    set_property(TARGET glad PROPERTY FOLDER "GameLib")
endif()

#------------------------------
# FMOD
# Sound library
#------------------------------
if(FMOD_SUPPORTED)
    add_subdirectory(fmod)
    #set_property(TARGET fmod PROPERTY FOLDER "GameLib")
endif()

#------------------------------
# ASSIMP
# Model loading library
#------------------------------
if(NOT TARGET assimp)
    IF (CMAKE_BUILD_TYPE MATCHES Debug AND NOT PLATFORM_ANDROID)
        set(BUILD_SHARED_LIBS ON)
    ELSE()
        set(BUILD_SHARED_LIBS OFF)
    ENDIF()

    set(BUILD_SHARED_LIBS ON)

    cmake_policy(SET CMP0077 NEW) # set() overrides option()

    set(ASM686 OFF)
    set(AMD64 ON)
    set(ASSIMP_BUILD_ZLIB ON)
    set(ASSIMP_BUILD_TESTS OFF)
    set(ZLIB_AMD64 ON)
    set(ASSIMP_NO_EXPORT ON)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
    set(ASSIMP_BUILD_FBX_IMPORTER ON)
    set(ASSIMP_BUILD_COLLADA_IMPORTER OFF)
    set(ASSIMP_BUILD_DRACO OFF)
    add_subdirectory(assimp)

    set_property(TARGET assimp PROPERTY FOLDER "GameLib")

    if(BUILD_SHARED_LIBS)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            if(WIN32)
                if(MSVC)
                    if(CMAKE_GENERATOR MATCHES "Visual Studio")
                        set(AURORA_DLL_TO_COPY
                                ${AURORA_DLL_TO_COPY}
                                ${CMAKE_CURRENT_BINARY_DIR}/assimp/bin/Debug/assimp-vc142-mtd.dll
                                CACHE INTERNAL "AURORA_DLL_TO_COPY")
                    else()
                        set(AURORA_DLL_TO_COPY
                                ${AURORA_DLL_TO_COPY}
                                ${CMAKE_CURRENT_BINARY_DIR}/assimp/bin/assimp-vc142-mtd.dll
                                CACHE INTERNAL "AURORA_DLL_TO_COPY")
                    endif()
                else(MSVC)
                    set(AURORA_DLL_TO_COPY
                            ${AURORA_DLL_TO_COPY}
                            ${CMAKE_CURRENT_BINARY_DIR}/assimp/bin/libassimpd.dll
                            CACHE INTERNAL "AURORA_DLL_TO_COPY")
                endif(MSVC)
            else(WIN32)
                set(AURORA_DLL_TO_COPY
                        ${AURORA_DLL_TO_COPY}
                        ${CMAKE_CURRENT_BINARY_DIR}/assimp/bin/libassimpd.so
                        CACHE INTERNAL "AURORA_DLL_TO_COPY")
            endif(WIN32)
        else()
            if(WIN32)
                if(MSVC)
                    if(CMAKE_GENERATOR MATCHES "Visual Studio")
                        set(AURORA_DLL_TO_COPY
                                ${AURORA_DLL_TO_COPY}
                                ${CMAKE_CURRENT_BINARY_DIR}/assimp/bin/Release/assimp-vc142-mt.dll
                                CACHE INTERNAL "AURORA_DLL_TO_COPY")
                    else()
                        set(AURORA_DLL_TO_COPY
                                ${AURORA_DLL_TO_COPY}
                                ${CMAKE_CURRENT_BINARY_DIR}/assimp/bin/assimp-vc142-mt.dll
                                CACHE INTERNAL "AURORA_DLL_TO_COPY")
                    endif()
                else(MSVC)
                    set(AURORA_DLL_TO_COPY
                            ${AURORA_DLL_TO_COPY}
                            ${CMAKE_CURRENT_BINARY_DIR}/assimp/bin/libassimp.dll
                            CACHE INTERNAL "AURORA_DLL_TO_COPY")
                endif(MSVC)
            else(WIN32)
                set(AURORA_DLL_TO_COPY
                        ${AURORA_DLL_TO_COPY}
                        ${CMAKE_CURRENT_BINARY_DIR}/assimp/bin/libassimp.so
                        CACHE INTERNAL "AURORA_DLL_TO_COPY")
            endif(WIN32)
        endif()
    endif()

    set(BUILD_SHARED_LIBS OFF)
endif()

#------------------------------
# ImGui
# Debug window render lib
#------------------------------
if(NOT TARGET ImGui)
    add_subdirectory(imgui)
    set_property(TARGET ImGui PROPERTY FOLDER "GameLib")
endif()

#------------------------------
# FreeType
# Font library
#------------------------------
if(NOT TARGET freetype)
    set(FT_WITH_BROTLI OFF)
    set(FT_WITH_HARFBUZZ OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB ON)
    set(CMAKE_DISABLE_FIND_PACKAGE_BZip2 ON)
    set(CMAKE_DISABLE_FIND_PACKAGE_PNG ON)
    set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON)
    set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec ON)
    add_subdirectory(freetype)
    set_property(TARGET freetype PROPERTY FOLDER "GameLib")
endif()

#------------------------------
# RmlUI
# UI render library
#------------------------------
if(NOT TARGET RmlUI)
    set(BUILD_SAMPLES ON)
    set(NO_FONT_INTERFACE_DEFAULT OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(CUSTOM_CONFIGURATION ON)
    set(CUSTOM_LINK_LIBRARIES freetype)
    add_subdirectory(RmlUI)
    set_property(TARGET RmlCore PROPERTY FOLDER "GameLib")
    set_property(TARGET RmlDebugger PROPERTY FOLDER "GameLib")
endif()

#------------------------------
# newton-dynamics
# Physics library
#------------------------------
if(NOT TARGET newton-dynamics)
    set(NEWTON_BUILD_SANDBOX_DEMOS ON)
    set(NEWTON_BUILD_SHARED_LIBS OFF)
    set(NEWTON_BUILD_SINGLE_THREADED OFF)
    set(NEWTON_DOUBLE_PRECISION OFF)
    set(NEWTON_ENABLE_AVX ON)
    set(NEWTON_BUILD_CREATE_SUB_PROJECTS OFF)
    set(NEWTON_STATIC_RUNTIME_LIBRARIES ON)
    add_subdirectory(newton-dynamics/newton-4.00)
    set_property(TARGET ndNewton PROPERTY FOLDER "GameLib")
endif()

#------------------------------
# RmlUI
# UI render library
#------------------------------
if(NOT TARGET tracy)
    add_subdirectory(tracy)
    set_property(TARGET TracyClient PROPERTY FOLDER "GameLib")
    if(WIN32)
        target_link_libraries(TracyClient PUBLIC Ws2_32 dbghelp)
        target_compile_definitions(TracyClient PUBLIC NOMINMAX)
    endif()
endif()

#------------------------------
# TinyGLTF
# Model loading library
#------------------------------
if(NOT TARGET tinygltf)
    set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF)
    set(TINYGLTF_BUILD_GL_EXAMPLES OFF)
    set(TINYGLTF_BUILD_VALIDATOR_EXAMPLE OFF)
    set(TINYGLTF_BUILD_BUILDER_EXAMPLE OFF)
    set(TINYGLTF_HEADER_ONLY OFF)
    set(TINYGLTF_INSTALL OFF)
    add_subdirectory(tinygltf)
endif()

#------------------------------
# EnTT
# ECS library
#------------------------------
if(NOT TARGET EnTT)
    add_subdirectory(entt)
    set_property(TARGET EnTT PROPERTY FOLDER "GameLib")
endif()