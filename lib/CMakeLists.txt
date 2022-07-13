#------------------------------
# GLFW
# Desktop Window Library
#------------------------------
if(NOT TARGET glfw AND NOT PLATFORM_ANDROID)
    set(GLFW_INSTALL OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_USE_HYBRID_HPG OFF)
    add_subdirectory(glfw)
    set_property(TARGET glfw PROPERTY FOLDER "Libs")
endif()

#--------------------------------
# nlohmann JSON
# JSON for Modern C++
#--------------------------------
if(NOT TARGET nlohmann_json)
    set(JSON_BuildTests OFF)
    set(JSON_Install OFF)
    add_subdirectory(nlohmann_json)
    set_property(TARGET nlohmann_json PROPERTY FOLDER "Libs")
endif()

#------------------------------
# GLFW
# Game mathematics library
#------------------------------
if(NOT TARGET glm)
    add_subdirectory(glm)
    set_property(TARGET glm PROPERTY FOLDER "Libs")
endif()

#------------------------------
# GLAD
# OpenGL
#------------------------------
if(NOT TARGET glad AND NOT PLATFORM_ANDROID)
    add_subdirectory(glad)
    set_property(TARGET glad PROPERTY FOLDER "Libs")
endif()

#------------------------------
# FMOD
# Sound library
#------------------------------
if(AU_FMOD_SOUND)
    add_subdirectory(fmod)
    #set_property(TARGET fmod PROPERTY FOLDER "Libs")
endif()

#------------------------------
# ImGui
# Debug window render lib
#------------------------------
if(NOT TARGET ImGui)
    add_subdirectory(imgui)
    set_property(TARGET ImGui PROPERTY FOLDER "Libs")
endif()

#------------------------------
# FreeType
# Font library
#------------------------------
if(NOT TARGET freetype)
    set(FT_WITH_BROTLI OFF)
    set(FT_WITH_HARFBUZZ OFF)
    set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB ON)
    set(CMAKE_DISABLE_FIND_PACKAGE_BZip2 ON)
    set(CMAKE_DISABLE_FIND_PACKAGE_PNG ON)
    set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON)
    set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec ON)
    add_subdirectory(freetype)
    set_property(TARGET freetype PROPERTY FOLDER "Libs")
endif()

#------------------------------
# RmlUI
# UI render library
#------------------------------
if(NOT TARGET RmlUI)
    set(BUILD_SAMPLES OFF)
    set(BUILD_TESTING OFF)
    set(NO_FONT_INTERFACE_DEFAULT OFF)
    #set(CUSTOM_CONFIGURATION ON)
    #set(CUSTOM_LINK_LIBRARIES freetype)
    set(NO_THIRDPARTY_CONTAINERS ON)
    add_subdirectory(RmlUI)
    set_property(TARGET RmlCore PROPERTY FOLDER "Libs")
    set_property(TARGET RmlDebugger PROPERTY FOLDER "Libs")
endif()

#------------------------------
# RmlUI
# UI render library
#------------------------------
if(NOT TARGET TracyClient)
    if(AU_CPU_PROFILE)
        set(TRACY_ENABLE ON)
    else()
        set(TRACY_ENABLE OFF)
    endif()

    set(BUILD_SHARED_LIBS_CACHE ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF)

    add_subdirectory(tracy)
    set_property(TARGET TracyClient PROPERTY FOLDER "GameLib")
    if(WIN32)
        target_link_libraries(TracyClient PUBLIC Ws2_32 dbghelp)
        target_compile_definitions(TracyClient PUBLIC NOMINMAX)
    endif()
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_CACHE})
    set_property(TARGET TracyClient PROPERTY FOLDER "Libs")
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

    set(BUILD_SHARED_LIBS_CACHE ${BUILD_SHARED_LIBS})

    if(NOT BUILD_SHARED_LIBS)
        set(BUILD_SHARED_LIBS ON)
    endif()

    set(ASM686 OFF)
    set(AMD64 ON)
    set(ASSIMP_BUILD_ZLIB ON)
    set(ASSIMP_BUILD_TESTS OFF)
    set(ZLIB_AMD64 ON)
    set(ASSIMP_NO_EXPORT ON)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
    set(ASSIMP_BUILD_FBX_IMPORTER ON)
    set(ASSIMP_BUILD_GLTF_IMPORTER OFF)
    set(ASSIMP_BUILD_COLLADA_IMPORTER OFF)
    set(ASSIMP_BUILD_DRACO OFF)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
    set(ASSIMP_INSTALL OFF)
    add_subdirectory(assimp)

    set_property(TARGET assimp PROPERTY FOLDER "Libs")
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_CACHE})
endif()

if(NOT TARGET stb_utils)
    add_subdirectory(stb)
    set_property(TARGET stb_utils PROPERTY FOLDER "Libs")
endif()