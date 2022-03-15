#pragma once

#include "../Base/Format.hpp"
#include "GL.hpp"

namespace Aurora
{
	struct FormatMapping
	{
		GraphicsFormat AbstractFormat;
		GLenum InternalFormat;
		GLenum BaseFormat;
		GLenum Type;
		uint32_t Components;
		uint32_t BytesPerPixel;
		bool IsDepthStencil;
	};

	// GraphicsFormat mapping table. The rows must be in the exactly same order as GraphicsFormat enum members are defined.
	// https://www.khronos.org/opengles/sdk/docs/man31/html/glTexImage2D.xhtml
	static const FormatMapping FormatMappings[] = {
			{ GraphicsFormat::Unknown,              0,                      0,                  0,                              0, 0, false },
			{ GraphicsFormat::R8_UINT,              GL_R8UI,                GL_RED_INTEGER,     GL_UNSIGNED_BYTE,               1, 1, false },
			{ GraphicsFormat::R8_UNORM,             GL_R8,                  GL_RED,             GL_UNSIGNED_BYTE,               1, 1, false },
			{ GraphicsFormat::RG8_UINT,             GL_RG8UI,               GL_RG_INTEGER,      GL_UNSIGNED_BYTE,               2, 2, false },
			{ GraphicsFormat::RG8_UNORM,            GL_RG8,                 GL_RG,              GL_UNSIGNED_BYTE,               2, 2, false },
			{ GraphicsFormat::R16_UINT,             GL_R16UI,               GL_RED_INTEGER,     GL_SHORT,                       1, 2, false },
			{ GraphicsFormat::R16_UNORM,            GL_R16,                 GL_RED,             GL_HALF_FLOAT,                  1, 2, false },
			{ GraphicsFormat::R16_FLOAT,            GL_R16F,                GL_RED,             GL_HALF_FLOAT,                  1, 2, false },
			{ GraphicsFormat::RGBA8_UNORM,          GL_RGBA8,               GL_RGBA,            GL_UNSIGNED_BYTE,               4, 4, false },
			{ GraphicsFormat::BGRA8_UNORM,          GL_RGBA8,               GL_BGRA,            GL_UNSIGNED_BYTE,               4, 4, false },
			{ GraphicsFormat::SRGBA8_UNORM,         GL_RGBA8,               GL_RGBA,            GL_UNSIGNED_BYTE,               4, 4, false },
			{ GraphicsFormat::R10G10B10A2_UNORM,    GL_RGB10_A2,            GL_RGBA,            GL_UNSIGNED_INT_2_10_10_10_REV, 4, 4, false },
			{ GraphicsFormat::R11G11B10_FLOAT,      GL_R11F_G11F_B10F,      GL_RGB,             GL_UNSIGNED_INT_10F_11F_11F_REV,4, 4, false },
			{ GraphicsFormat::RG16_UINT,            GL_RG16UI,              GL_RG_INTEGER,      GL_SHORT,                       2, 4, false },
			{ GraphicsFormat::RG16_FLOAT,           GL_RG16F,               GL_RG,              GL_HALF_FLOAT,                  2, 4, false },
			{ GraphicsFormat::R32_UINT,             GL_R32UI,               GL_RED_INTEGER,     GL_UNSIGNED_INT,                1, 4, false },
			{ GraphicsFormat::R32_FLOAT,            GL_R32F,                GL_RED,             GL_FLOAT,                       1, 4, false },
			{ GraphicsFormat::RGB16_FLOAT,          GL_RGB16F,              GL_RGB,             GL_FLOAT,                       3, 8, false },
			{ GraphicsFormat::RGBA16_FLOAT,         GL_RGBA16F,             GL_RGBA,            GL_HALF_FLOAT,                  4, 8, false },
			{ GraphicsFormat::RGBA16_UNORM,         GL_RGBA16,              GL_RGBA,            GL_UNSIGNED_INT,                4, 8, false },
			{ GraphicsFormat::RGBA16_SNORM,         GL_RGBA16_SNORM,        GL_RGBA,            GL_INT,                         4, 8, false },
			{ GraphicsFormat::RG32_UINT,            GL_RG32UI,              GL_RG_INTEGER,      GL_UNSIGNED_INT,                2, 8, false },
			{ GraphicsFormat::RG32_FLOAT,           GL_RG32F,               GL_RG,              GL_FLOAT,                       2, 8, false },
			{ GraphicsFormat::RGB8_UNORM,           GL_RGB8,                GL_RGB,             GL_UNSIGNED_BYTE,                3, 8, false },
			{ GraphicsFormat::RGB32_UINT,           GL_RGB32UI,             GL_RGB_INTEGER,     GL_UNSIGNED_INT,                3, 12, false },
			{ GraphicsFormat::RGB32_FLOAT,          GL_RGB32F,              GL_RGB,             GL_FLOAT,                       3, 12, false },
			{ GraphicsFormat::RGBA16_UINT,          GL_RGBA16UI,            GL_RGBA_INTEGER,    GL_UNSIGNED_INT,                4, 16, false },
			{ GraphicsFormat::RGBA32_UINT,          GL_RGBA32UI,            GL_RGBA_INTEGER,    GL_UNSIGNED_INT,                4, 16, false },
			{ GraphicsFormat::RGBA32_FLOAT,         GL_RGBA32F,             GL_RGBA,            GL_FLOAT,                       4, 16, false },
			{ GraphicsFormat::D16,                  GL_DEPTH_COMPONENT16,   GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,              1, 2, true },
			{ GraphicsFormat::D24S8,                GL_DEPTH24_STENCIL8,    GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8,           2, 4, true },
			{ GraphicsFormat::X24G8_UINT,           GL_DEPTH24_STENCIL8,    GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8,           2, 4, true },
			{ GraphicsFormat::D32,                  GL_DEPTH_COMPONENT32,   GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,                1, 4, true },
	};

	static const FormatMapping& GetFormatMapping(const GraphicsFormat& abstractFormat)
	{
		auto index = (uint8_t)abstractFormat;
		const FormatMapping& mapping = FormatMappings[index];
		assert(mapping.AbstractFormat == abstractFormat);
		return mapping;
	}
}