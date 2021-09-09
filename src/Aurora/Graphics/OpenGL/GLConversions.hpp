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
	GLenum ConvertStencilOp(EStencilOp value);
	GLenum ConvertComparisonFunc(EComparisonFunc value);
	GLenum ConvertPrimType(EPrimitiveType primType);
	GLint ConvertWrapMode(EWrapMode wrapMode);
	GLenum ConvertBlendValue(EBlendValue value);
	GLenum ConvertBlendOp(EBlendOp value);
	GLenum ConvertBufferType(EBufferType bufferType);
	GLenum ConvertUsage(EBufferUsage usage);
	GLenum ConvertIndexBufferFormat(EIndexBufferFormat format);
	GLenum ConvertBufferAccess(EBufferAccess bufferAccess);
}