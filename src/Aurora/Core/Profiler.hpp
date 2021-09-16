#pragma once

#include <Aurora/Graphics/OpenGL/GL.hpp>
#include <TracyOpenGL.hpp>

#define CAT_(a, b) a ## b
#define CAT(a, b) CAT_(a, b)

#if defined(AURORA_OPENGL)
#include "Aurora/Graphics/OpenGL/GLRenderGroupScope.hpp"
#define GPU_DEBUG_SCOPE(name) ::Aurora::GLRenderGroupScope CAT(_GPU_Debug_Scope_, __LINE__)(name);
#else
#define GPU_DEBUG_SCOPE(name)
#endif

#define CPU_DEBUG_SCOPE(name) ZoneNamedN(CAT(_GPU_Debug_Scope_, __LINE__), name, true)