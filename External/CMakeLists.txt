set(DILIGENT_BUILD_SAMPLES OFF)
set(DILIGENT_BUILD_DEMOS OFF)
set(DILIGENT_BUILD_UNITY_PLUGIN OFF)
set(DILIGENT_INSTALL_TOOLS OFF)
add_subdirectory(DiligentEngine)

#------------------------------
# GLFW
# Desktop Window Library
#------------------------------
set(GLFW_INSTALL OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(glfw)

add_subdirectory(glm)

#------------------------------
# ASSIMP
# Model loading library
#------------------------------
IF (CMAKE_BUILD_TYPE MATCHES Debug)
    set(BUILD_SHARED_LIBS ON)
ELSE()
    set(BUILD_SHARED_LIBS OFF)
ENDIF()

set(ASM686 OFF)
set(AMD64 ON)
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ZLIB_AMD64 ON)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
set(ASSIMP_BUILD_COLLADA_IMPORTER ON)
add_subdirectory(assimp)
if(MINGW)
    target_link_libraries(assimp -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
endif()