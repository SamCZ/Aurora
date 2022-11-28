project(AuroraCore)

file(GLOB_RECURSE PROJECT_SOURCE src/*.hpp src/*.cpp src/*.h src/*.c)

if(NOT BUILD_SHARED_LIBS)
	add_library(Aurora STATIC ${PROJECT_SOURCE})
	target_compile_definitions(Aurora PUBLIC AU_SHARED=0)
else()
	add_library(Aurora SHARED ${PROJECT_SOURCE})
	target_compile_definitions(Aurora PUBLIC AU_SHARED=1)
	target_compile_definitions(Aurora PRIVATE AU_EXPORT=1)
endif()

if(AU_IN_PROJECT_ASSETS)
	target_compile_definitions(Aurora PUBLIC AU_IN_PROJECT_ASSETS=1)
else()
	target_compile_definitions(Aurora PUBLIC AU_IN_PROJECT_ASSETS=0)
endif()

target_include_directories(Aurora PUBLIC src)
target_include_directories(Aurora PUBLIC ../Assets)

target_compile_definitions(Aurora PUBLIC GLFW_INCLUDE_NONE=1)
target_compile_definitions(Aurora PUBLIC GLFW_ENABLED=1)
#target_compile_definitions(Aurora PUBLIC RMLUI_STATIC_LIB=1)
target_compile_definitions(Aurora PUBLIC AURORA_OPENGL=1)
target_compile_definitions(Aurora PUBLIC SHADER_ENGINE_SIDE=1)
target_compile_definitions(Aurora PUBLIC AURORA_PROJECT_DIR="${AU_ROOT_SOURCE_DIR}")

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

target_link_libraries(Aurora PUBLIC glad glfw glm nlohmann_json TracyClient ImGui assimp stb_utils)
target_link_libraries(Aurora PUBLIC RmlCore RmlDebugger)

if (AU_FMOD_SOUND)
	target_compile_definitions(Aurora PUBLIC AU_FMOD_SOUND=1)
	target_include_directories(Aurora PUBLIC ${AURORA_ADD_INCLUDE_DIRS})
	target_link_libraries(Aurora PUBLIC ${FMOD_CORE_LIBRARY})
	target_link_libraries(Aurora PUBLIC ${FMOD_STUDIO_LIBRARY})

	add_custom_command(TARGET Aurora POST_BUILD # Adds a post-build event to MyTest
					COMMAND ${CMAKE_COMMAND} -E copy_if_different # which executes "cmake - E copy_if_different..."
					"${FMOD_CORE_LIBRARY_DLL}" # <--this is in-file
					$<TARGET_FILE_DIR:Aurora>) # <--this is out-file path

	add_custom_command(TARGET Aurora POST_BUILD # Adds a post-build event to MyTest
					COMMAND ${CMAKE_COMMAND} -E copy_if_different # which executes "cmake - E copy_if_different..."
					"${FMOD_STUDIO_LIBRARY_DLL}" # <--this is in-file
					$<TARGET_FILE_DIR:Aurora>) # <--this is out-file path
endif()