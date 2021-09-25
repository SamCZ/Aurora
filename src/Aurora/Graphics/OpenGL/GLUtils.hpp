#pragma once

#include "Aurora/Logger/Logger.hpp"
#include <glm/glm.hpp>
#include <cstring>

#include "GL.hpp"

#include "../Base/Format.hpp"

// This needs to added to cmake option as some render debug
#ifndef OPENGL_ERROR_CHECKING
#define OPENGL_ERROR_CHECKING
#endif

#ifdef OPENGL_ERROR_CHECKING
#ifndef CHECK_GL_ERROR_ARG
#define CHECK_GL_ERROR_ARG(...)                                                                                              \
    do                                                                                                                   \
    {                                                                                                                    \
        auto err = glGetError();                                                                                         \
        if (err != GL_NO_ERROR)                                                                                          \
        {                                                                                                                \
        	AU_LOG_ERROR(__VA_ARGS__, "\nGL Error Code: ", err);                                                         \
        }                                                                                                                \
    } while (false);
#define CHECK_GL_ERROR(...)                                                                                                 \
    do                                                                                                                   \
    {                                                                                                                    \
        auto err = glGetError();                                                                                         \
        if (err != GL_NO_ERROR)                                                                                          \
        {                                                                                                                \
        	AU_LOG_ERROR("\nGL Error Code: ", err);                                                                            \
        	throw std::runtime_error("OpenGL Error !");                                                                 \
        }                                                                                                                \
    } while (false);
#endif

#ifndef CHECK_GL_ERROR_AND_THROW
#define CHECK_GL_ERROR_AND_THROW(...)                                                                                   \
    do                                                                                                                  \
    {                                                                                                                   \
        auto err = glGetError();                                                                                        \
        if (err != GL_NO_ERROR)                                                                                         \
        {                                                                                                               \
        	AU_LOG_ERROR(__VA_ARGS__, "\nGL Error Code: ", err);                                                        \
        	throw std::runtime_error("OpenGL Error !");                                                                 \
        }                                                                                                               \
    } while (false);
#endif
#else
#define CHECK_GL_ERROR_ARG(...)
#define CHECK_GL_ERROR(...)
#define CHECK_GL_ERROR_AND_THROW(...)
#endif

namespace Aurora
{
	static void RemoveArrayBrackets(char* Str)
	{
		auto* OpenBacketPtr = strchr(Str, '[');
		if (OpenBacketPtr != nullptr)
			*OpenBacketPtr = 0;
	}

	struct GLType
	{
		std::string Name;
		size_t Size;
		GraphicsFormat Format;
	};

	static GLType GetGLType(GLenum glType)
	{
#define GET_GL_TYPE(glType, type, components, format) case glType: return {#glType, (sizeof(type) * (components)), format};
		switch (glType) {
			GET_GL_TYPE(GL_FLOAT, float, 1, GraphicsFormat::R32_FLOAT)
			GET_GL_TYPE(GL_INT, int32_t, 1, GraphicsFormat::Unknown)
			GET_GL_TYPE(GL_UNSIGNED_INT, uint32_t, 1, GraphicsFormat::R8_UINT)
			GET_GL_TYPE(GL_BOOL, int32_t, 1, GraphicsFormat::Unknown)

			GET_GL_TYPE(GL_FLOAT_VEC2, float, 2, GraphicsFormat::RG32_FLOAT)
			GET_GL_TYPE(GL_FLOAT_VEC3, float, 3, GraphicsFormat::RGB32_FLOAT)
			GET_GL_TYPE(GL_FLOAT_VEC4, float, 4, GraphicsFormat::RGBA32_FLOAT)

			GET_GL_TYPE(GL_INT_VEC2, int32_t, 2, GraphicsFormat::Unknown)
			GET_GL_TYPE(GL_INT_VEC3, int32_t, 3, GraphicsFormat::Unknown)
			GET_GL_TYPE(GL_INT_VEC4, int32_t, 4, GraphicsFormat::Unknown)

			GET_GL_TYPE(GL_UNSIGNED_INT_VEC2, uint32_t, 2, GraphicsFormat::RG32_UINT)
			GET_GL_TYPE(GL_UNSIGNED_INT_VEC3, uint32_t, 3, GraphicsFormat::RGB32_UINT)
			GET_GL_TYPE(GL_UNSIGNED_INT_VEC4, uint32_t, 4, GraphicsFormat::RGBA32_UINT)

			GET_GL_TYPE(GL_BOOL_VEC2, int32_t, 2, GraphicsFormat::Unknown)
			GET_GL_TYPE(GL_BOOL_VEC3, int32_t, 3, GraphicsFormat::Unknown)
			GET_GL_TYPE(GL_BOOL_VEC4, int32_t, 4, GraphicsFormat::Unknown)

			GET_GL_TYPE(GL_FLOAT_MAT3, float, 9, GraphicsFormat::Unknown)
			GET_GL_TYPE(GL_FLOAT_MAT4, float, 16, GraphicsFormat::Unknown)

			GET_GL_TYPE(GL_FLOAT_MAT2x3, float, 2 * 3, GraphicsFormat::Unknown)
			GET_GL_TYPE(GL_FLOAT_MAT2x4, float, 2 * 4, GraphicsFormat::Unknown)

			GET_GL_TYPE(GL_FLOAT_MAT3x2, float, 3 * 2, GraphicsFormat::Unknown)
			GET_GL_TYPE(GL_FLOAT_MAT3x4, float, 3 * 4, GraphicsFormat::Unknown)

			GET_GL_TYPE(GL_FLOAT_MAT4x2, float, 4 * 2, GraphicsFormat::Unknown)
			GET_GL_TYPE(GL_FLOAT_MAT4x3, float, 4 * 3, GraphicsFormat::Unknown)

			default:
				AU_LOG_FATAL("Unknown opengl type for size conversion: ", glType);
				return {};
		}

#undef GL_TYPE
	}

	static size_t GetOpenGLDataTypeSize(GLenum dataType)
	{
		return GetGLType(dataType).Size;
	}
}