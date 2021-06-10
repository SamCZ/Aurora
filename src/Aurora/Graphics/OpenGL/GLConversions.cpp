#include "GLConversions.hpp"

#include "Aurora/Logger/Logger.hpp"

namespace Aurora
{
	GLenum ConvertStencilOp(EStencilOp value)
	{
		switch (value)
		{
			case EStencilOp::Keep:
				return GL_KEEP;
			case EStencilOp::Zero:
				return GL_ZERO;
			case EStencilOp::Replace:
				return  GL_REPLACE;
			case EStencilOp::IncrSat:
				return GL_INCR_WRAP;
			case EStencilOp::DecrSat:
				return GL_DECR_WRAP;
			case EStencilOp::Invert:
				return GL_INVERT;
			case EStencilOp::Increment:
				return GL_INCR;
			case EStencilOp::Decrement:
				return GL_DECR;
			default: AU_LOG_FATAL("Unknown stencil op %d", (int)value);
				return GL_KEEP;
		}
	}

	GLenum ConvertComparisonFunc(EComparisonFunc value)
	{
		switch (value)
		{
			case EComparisonFunc::Never:
				return GL_NEVER;
			case EComparisonFunc::Less:
				return GL_LESS;
			case EComparisonFunc::Equal:
				return GL_EQUAL;
			case EComparisonFunc::LessEqual:
				return GL_LEQUAL;
			case EComparisonFunc::Greater:
				return GL_GREATER;
			case EComparisonFunc::NotEqual:
				return GL_NOTEQUAL;
			case EComparisonFunc::GreaterEqual:
				return GL_GEQUAL;
			case EComparisonFunc::Always:
				return GL_ALWAYS;
			default: AU_LOG_FATAL("Unknown comparison func %d", (int)value);
				return GL_NEVER;
		}
	}

	GLenum ConvertPrimType(PrimitiveType primType)
	{
		switch (primType)
		{
			case PrimitiveType::PointList:
				return GL_POINTS;
			case PrimitiveType::TriangleList:
				return GL_TRIANGLES;
			case PrimitiveType::TriangleStrip:
				return GL_TRIANGLE_STRIP;
			default: AU_LOG_FATAL("Unsupported primitive type %d", (int)primType);
		}

		return GL_NONE;
	}

	GLint ConvertWrapMode(EWrapMode wrapMode)
	{
		switch (wrapMode)
		{
			case EWrapMode::Clamp:
				return GL_CLAMP_TO_EDGE;
			case EWrapMode::Wrap:
				return GL_REPEAT;
			case EWrapMode::Border:
#if PLATFORM_ANDROID
				return GL_NONE;
#else
				return GL_CLAMP_TO_BORDER;
#endif
			default: AU_LOG_FATAL("Unknown wrap mode specified %d", (int)wrapMode);
				return GL_CLAMP_TO_EDGE;
				break;
		}
	}

	GLenum ConvertBlendValue(EBlendValue value)
	{
		switch (value)
		{
			case EBlendValue::Zero:
				return GL_ZERO;
			case EBlendValue::One:
				return GL_ONE;
			case EBlendValue::SrcColor:
				return GL_SRC_COLOR;
			case EBlendValue::InvSrcColor:
				return GL_ONE_MINUS_SRC_COLOR;
			case EBlendValue::SrcAlpha:
				return GL_SRC_ALPHA;
			case EBlendValue::InvSrcAlpha:
				return GL_ONE_MINUS_SRC_ALPHA;
			case EBlendValue::DestAlpha:
				return GL_DST_ALPHA;
			case EBlendValue::InvDestAlpha:
				return GL_ONE_MINUS_DST_ALPHA;
			case EBlendValue::DestColor:
				return GL_DST_COLOR;
			case EBlendValue::InvDestColor:
				return GL_ONE_MINUS_DST_COLOR;
			case EBlendValue::SrcAlphaSat:
				return GL_SRC_ALPHA_SATURATE;
			case EBlendValue::BlendFactor:
				return GL_CONSTANT_ALPHA;
			case EBlendValue::InvBlendFactor:
				return GL_ONE_MINUS_CONSTANT_ALPHA;
#if !PLATFORM_ANDROID
			case EBlendValue::Src1Color:
				return GL_SRC1_COLOR;
			case EBlendValue::InvSrc1Color:
				return GL_ONE_MINUS_SRC1_COLOR;
			case EBlendValue::Src1Alpha:
				return GL_SRC1_ALPHA;
			case EBlendValue::InvSrc1Alpha:
				return GL_ONE_MINUS_SRC1_ALPHA;
#endif
			default: AU_LOG_FATAL("Unknown blend value %d", (int)value);
				return GL_ZERO;
		}
	}

	GLenum ConvertBlendOp(EBlendOp value)
	{
		switch (value)
		{
			case EBlendOp::Add:
				return GL_FUNC_ADD;
			case EBlendOp::Subtract:
				return GL_FUNC_SUBTRACT;
			case EBlendOp::RevSubtract:
				return GL_FUNC_REVERSE_SUBTRACT;
			case EBlendOp::Min:
				return GL_MIN;
			case EBlendOp::Max:
				return GL_MAX;
			default: AU_LOG_FATAL("Unknown blend op %d", (int)value);
				return GL_FUNC_ADD;
		}
	}

	GLenum ConvertBufferType(EBufferType bufferType)
	{
		switch (bufferType) {

			case EBufferType::VertexBuffer: return GL_ARRAY_BUFFER;
			case EBufferType::IndexBuffer: return GL_ELEMENT_ARRAY_BUFFER;
			case EBufferType::UniformBuffer: return GL_UNIFORM_BUFFER;
			case EBufferType::ShaderStorageBuffer: return GL_SHADER_STORAGE_BUFFER;
			case EBufferType::TextureBuffer: return GL_TEXTURE_BUFFER;
			default:
			case EBufferType::Unknown: {
				AU_LOG_WARNING("Unknown buffer type !");
				throw;
				return GL_NONE;
			}
		}
	}

	GLenum ConvertUsage(EBufferUsage usage)
	{
		switch(usage) {
			case EBufferUsage::StreamDraw: return GL_STREAM_DRAW;
			case EBufferUsage::StreamRead: return GL_STREAM_READ;
			case EBufferUsage::StreamCopy: return GL_STREAM_COPY;

			case EBufferUsage::StaticDraw: return GL_STATIC_DRAW;
			case EBufferUsage::StaticRead: return GL_STATIC_READ;
			case EBufferUsage::StaticCopy: return GL_STATIC_COPY;

			case EBufferUsage::DynamicDraw: return GL_DYNAMIC_DRAW;
			case EBufferUsage::DynamicRead: return GL_DYNAMIC_READ;
			case EBufferUsage::DynamicCopy: return GL_DYNAMIC_COPY;
			default:
			case EBufferUsage::Unknown: {
				AU_LOG_WARNING("Unknown usage type !");
				throw;
				return GL_NONE;
			}
		}
	}
}