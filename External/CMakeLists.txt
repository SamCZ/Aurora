set(DILIGENT_BUILD_SAMPLES OFF)
set(DILIGENT_BUILD_DEMOS OFF)
set(DILIGENT_BUILD_UNITY_PLUGIN OFF)
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