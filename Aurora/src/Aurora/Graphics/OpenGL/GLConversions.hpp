#pragma once

#include "GL.hpp"

#include "../Base/BlendState.hpp"
#include "../Base/FDepthStencilState.hpp"
#include "../Base/PrimitiveType.hpp"
#include "../Base/RasterState.hpp"
#include "../Base/Sampler.hpp"
#include "../Base/Buffer.hpp"

namespace Aurora
{
	AU_API GLenum ConvertStencilOp(EStencilOp value);
	AU_API GLenum ConvertComparisonFunc(EComparisonFunc value);
	AU_API GLenum ConvertPrimType(EPrimitiveType primType);
	AU_API GLint ConvertWrapMode(EWrapMode wrapMode);
	AU_API GLenum ConvertBlendValue(EBlendValue value);
	AU_API GLenum ConvertBlendOp(EBlendOp value);
	AU_API GLenum ConvertBufferType(EBufferType bufferType);
	AU_API GLenum ConvertUsage(EBufferUsage usage);
	AU_API GLenum ConvertIndexBufferFormat(EIndexBufferFormat format);
	AU_API GLenum ConvertBufferAccess(EBufferAccess bufferAccess);
}