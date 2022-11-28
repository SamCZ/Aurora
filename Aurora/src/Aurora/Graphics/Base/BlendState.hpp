#pragma once

#include "Aurora/Graphics/Color.hpp"

namespace Aurora
{
	enum class EBlendValue : uint8_t
	{
		Zero = 1,
		One = 2,
		SrcColor = 3,
		InvSrcColor = 4,
		SrcAlpha = 5,
		InvSrcAlpha = 6,
		DestAlpha = 7,
		InvDestAlpha = 8,
		DestColor = 9,
		InvDestColor = 10,
		SrcAlphaSat = 11,
		BlendFactor = 14,
		InvBlendFactor = 15,
		Src1Color = 16,
		InvSrc1Color = 17,
		Src1Alpha = 18,
		InvSrc1Alpha = 19
	};

	enum class EBlendOp : uint8_t
	{
		Add = 1,
		Subtract = 2,
		RevSubtract = 3,
		Min = 4,
		Max = 5
	};

	enum class EColorMask : uint8_t
	{
		Red = 1,
		Green = 2,
		Blue = 4,
		Alpha = 8,
		All = 0xF
	};

	struct FBlendState
	{
		bool Enabled;
		EBlendValue SrcBlend;
		EBlendValue DestBlend;

		EBlendOp BlendOp;
		EBlendValue SrcBlendAlpha;
		EBlendValue DestBlendAlpha;
		EBlendOp BlendOpAlpha;
		EColorMask ColorWriteEnable;
		Color BlendFactor;
		bool AlphaToCoverage;

		FBlendState()
		: Enabled(false),
		  SrcBlend(EBlendValue::One),
		  DestBlend(EBlendValue::One),
		  BlendOp(EBlendOp::Add),
		  SrcBlendAlpha(EBlendValue::One),
		  DestBlendAlpha(EBlendValue::One),
		  BlendOpAlpha(EBlendOp::Add),
		  ColorWriteEnable(EColorMask::All),
		  BlendFactor(0, 0, 0, 0),
		  AlphaToCoverage(false) { }
	};
}