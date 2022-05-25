#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#if PLATFORM_ANDROIDS
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#include <glad/gl.h>
#endif

#include "Aurora/Core/Library.hpp"