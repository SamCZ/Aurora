#pragma once

#include "Aurora/Graphics/OpenGL/GL.hpp"

#if AU_TRACY_ENABLED
#include <TracyOpenGL.hpp>
#endif

#define CAT_(a, b) a ## b
#define CAT(a, b) CAT_(a, b)

#if defined(AURORA_OPENGL) && AU_GPU_PROFILE == 1
#include "Aurora/Graphics/OpenGL/GLRenderGroupScope.hpp"
#define GPU_DEBUG_SCOPE(name) ::Aurora::GLRenderGroupScope CAT(_GPU_Debug_Scope_, __LINE__)(name)
#else
#define GPU_DEBUG_SCOPE(name)
#endif

#if AU_TRACY_ENABLED
#define CPU_DEBUG_SCOPE(name) ZoneNamedN(CAT(_GPU_Debug_Scope_, __LINE__), name, true)
#else
#define CPU_DEBUG_SCOPE(name)
#endif