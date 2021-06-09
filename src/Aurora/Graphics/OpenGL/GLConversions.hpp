#pragma once

#include "GL.hpp"

#include "../Base/BlendState.hpp"
#include "../Base/DepthStencilState.hpp"
#include "../Base/PrimitiveType.hpp"
#include "../Base/RasterState.hpp"
#include "../Base/Sampler.hpp"

namespace Aurora
{
	static GLenum ConvertStencilOp(EStencilOp value);
	static GLenum ConvertComparisonFunc(EComparisonFunc value);
	static GLenum ConvertPrimType(PrimitiveType primType);
	static GLenum ConvertWrapMode(EWrapMode wrapMode);
	static GLenum ConvertBlendValue(EBlendValue value);
	static GLenum ConvertBlendOp(EBlendOp value);
}