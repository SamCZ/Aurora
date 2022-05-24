#pragma once

#include "Aurora/Logger/Logger.hpp"
#include <glm/glm.hpp>
#include <cstring>

#include "GL.hpp"

#include "../Base/Format.hpp"
#include "../Base/ShaderBase.hpp"

static const char* GLErrorToString(GLenum e)
{
	switch (e)
	{
		case GL_NO_ERROR:
			return "GL_NO_ERROR";
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		default:
			return "UNKNOWN_ERROR";
	}
}

#if OPENGL_ERROR_CHECKING
#ifndef CHECK_GL_ERROR_ARG
#define CHECK_GL_ERROR_ARG(...)                                                                                              \
    do                                                                                                                   \
    {                                                                                                                    \
        auto err = glGetError();                                                                                         \
        if (err != GL_NO_ERROR)                                                                                          \
        {                                                                                                                \
        	AU_LOG_ERROR(__VA_ARGS__, "\nGL Error Code: ", GLErrorToString(err));                                                         \
        }                                                                                                                \
    } while (false);
#define CHECK_GL_ERROR(...)                                                                                                 \
    do                                                                                                                   \
    {                                                                                                                    \
        auto err = glGetError();                                                                                         \
        if (err != GL_NO_ERROR)                                                                                          \
        {                                                                                                                \
        	AU_LOG_ERROR("\nGL Error Code: ", GLErrorToString(err));                                                     \
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
        	AU_LOG_ERROR(__VA_ARGS__, "\nGL Error Code: ", GLErrorToString(err));                                       \
        }                                                                                                               \
    } while (false);
#endif
#else
#define CHECK_GL_ERROR_ARG(...) do {} while(false)
#define CHECK_GL_ERROR(...) do {} while(false)
#define CHECK_GL_ERROR_AND_THROW(...) do {} while(false)
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
		uint8_t ComponentCount;
		GraphicsFormat Format;
	};

	static GLType GetGLType(GLenum glType)
	{
#define GET_GL_TYPE(glType, type, components, format) case glType: return {#glType, (sizeof(type) * (components)), components, format};
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


			GET_GL_TYPE(GL_UNSIGNED_INT64_NV, uint64_t , 1, GraphicsFormat::Unknown)
			GET_GL_TYPE(GL_UNSIGNED_INT64_AMD, uint64_t , 1, GraphicsFormat::Unknown)

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

	static VarType GetGLVarType(GLenum glType)
	{
#define GET_GL_VARTYPE(glType, format) case glType: return format;
		switch (glType) {
			GET_GL_VARTYPE(GL_FLOAT, VarType::Float)
			GET_GL_VARTYPE(GL_INT, VarType::Int)
			GET_GL_VARTYPE(GL_UNSIGNED_INT, VarType::UnsignedInt)
			GET_GL_VARTYPE(GL_BOOL, VarType::Bool)

			GET_GL_VARTYPE(GL_FLOAT_VEC2, VarType::Vec2)
			GET_GL_VARTYPE(GL_FLOAT_VEC3, VarType::Vec3)
			GET_GL_VARTYPE(GL_FLOAT_VEC4, VarType::Vec4)

			GET_GL_VARTYPE(GL_INT_VEC2, VarType::IVec2)
			GET_GL_VARTYPE(GL_INT_VEC3, VarType::IVec3)
			GET_GL_VARTYPE(GL_INT_VEC4, VarType::IVec4)

			GET_GL_VARTYPE(GL_UNSIGNED_INT_VEC2, VarType::UIVec2)
			GET_GL_VARTYPE(GL_UNSIGNED_INT_VEC3, VarType::UIVec3)
			GET_GL_VARTYPE(GL_UNSIGNED_INT_VEC4, VarType::UIVec4)

			GET_GL_VARTYPE(GL_BOOL_VEC2, VarType::BoolVec2)
			GET_GL_VARTYPE(GL_BOOL_VEC3, VarType::BoolVec3)
			GET_GL_VARTYPE(GL_BOOL_VEC4, VarType::BoolVec4)

			GET_GL_VARTYPE(GL_FLOAT_MAT3, VarType::Mat3x3)
			GET_GL_VARTYPE(GL_FLOAT_MAT4, VarType::Mat4x4)

			GET_GL_VARTYPE(GL_FLOAT_MAT2x3, VarType::Mat2x3)
			GET_GL_VARTYPE(GL_FLOAT_MAT2x4, VarType::Mat2x4)

			GET_GL_VARTYPE(GL_FLOAT_MAT3x2, VarType::Mat3x2)
			GET_GL_VARTYPE(GL_FLOAT_MAT3x4, VarType::Mat3x4)

			GET_GL_VARTYPE(GL_FLOAT_MAT4x2, VarType::Mat4x2)
			GET_GL_VARTYPE(GL_FLOAT_MAT4x3, VarType::Mat4x3)

			default:
			AU_LOG_FATAL("Unknown opengl type for size conversion: ", glType);
				return {};
		}

#undef GL_TYPE
	}
}