#------------------------------
# Vulkan
# Render pipeline
#------------------------------
add_subdirectory(Vulkan)

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

#------------------------------
# glm
# Game math library
#------------------------------
add_subdirectory(glm)