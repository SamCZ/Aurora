#pragma once

#include <cstring>

namespace Aurora
{
#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#endif
	enum class GraphicsFormat : uint8_t
	{
		Unknown = 0,
		R8_UINT,
		R8_UNORM,
		RG8_UINT,
		RG8_UNORM,
		R16_UINT,
		R16_UNORM,
		R16_FLOAT,
		RGBA8_UNORM,
		BGRA8_UNORM,
		SRGBA8_UNORM,
		R10G10B10A2_UNORM,
		R11G11B10_FLOAT,
		RG16_UINT,
		RG16_FLOAT,
		R32_UINT,
		R32_FLOAT,
		RGB16_FLOAT,
		RGBA16_FLOAT,
		RGBA16_UNORM,
		RGBA16_SNORM,
		RG32_UINT,
		RG32_FLOAT,
		RGB32_UINT,
		RGB32_FLOAT,
		RGBA16_UINT,
		RGBA32_UINT,
		RGBA32_FLOAT,
		D16,
		D24S8,
		X24G8_UINT,
		D32,
	};
#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif
}