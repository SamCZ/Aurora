#pragma once

#include "Aurora/Core/Common.hpp"

namespace Aurora
{
	AENUM()
	enum class ETextureCompressionSettings
	{
		Default             AMETA(DisplayName = "Default (DXT1/5, BC1/3 on DX11)"),
		NormalMap           AMETA(DisplayName = "Normalmap (DXT5, BC5 on DX11)"),
		Masks               AMETA(DisplayName = "Masks (no sRGB)"),
		Grayscale           AMETA(DisplayName = "Grayscale (R8, RGB8 sRGB)"),
		DisplacementMap     AMETA(DisplayName = "DisplacementMap (8/16bit)"),
		VectorDisplacementMap       AMETA(DisplayName = "VectorDisplacementMap (RGBA8)"),
		HDR                         AMETA(DisplayName = "HDR (RGB, no sRGB)"),
		EditorIcon                  AMETA(DisplayName = "(RGBA)"),
		Alpha                       AMETA(DisplayName = "Alpha (no sRGB, BC4 on DX11)"),
		HDR_Compressed              AMETA(DisplayName = "HDRCompressed (RGB, BC6H, DX11)"),
		BC7                         AMETA(DispalyName = "BC7 (DX11, optional A)")
	};

	AENUM()
	enum class ETextureSourceEncoding
	{
		None    = 0 AMETA(DisplayName = "Defaults", ToolTip = "The source encoding is assumed to match the state of the sRGB checkbox parameter."),
		Linear  = 1 AMETA(DisplayName = "Linear", ToolTip = "The source encoding is considered linear (before optional sRGB encoding is applied)."),
		sRGB    = 2 AMETA(DisplayName = "sRGB", ToolTip = "sRGB source encoding to be linearized (before optional sRGB encoding is applied).")
	};

	ACLASS()
	class TextureAsset
	{

	};
}