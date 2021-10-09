#pragma once

#if PLATFORM_ANDROID
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#include <glad/gl.h>
#endif