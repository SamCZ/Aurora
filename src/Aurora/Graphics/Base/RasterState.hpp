#pragma once

#include <cstdint>

namespace Aurora
{
	enum class EFillMode : uint8_t
	{
		Solid = 0,
		Line
	};

	enum class ECullMode : uint8_t
	{
		Back = 0,
		Front,
		None
	};

	struct FRasterState
	{
		EFillMode    FillMode;
		ECullMode    CullMode;
		bool FrontCounterClockwise;
		bool DepthClipEnable;
		bool ScissorEnable;
		bool MultisampleEnable;
		int DepthBias;
		float DepthBiasClamp;
		float SlopeScaledDepthBias;

		FRasterState()
		: FillMode(EFillMode::Solid),
		  CullMode(ECullMode::Front),
		  FrontCounterClockwise(false),
		  DepthClipEnable(true),
		  ScissorEnable(false),
		  MultisampleEnable(false),
		  DepthBias(0),
		  DepthBiasClamp(0),
		  SlopeScaledDepthBias(0) { }
	};
}