#pragma once

namespace Aurora
{
	enum class FillMode : unsigned char
	{
		Solid,
		Line
	};

	enum class CullMode : unsigned char
	{
		Back,
		Front,
		None
	};

	struct FRasterState
	{
		FillMode    FillMode;
		CullMode    CullMode;
		bool FrontCounterClockwise{};
		bool DepthClipEnable;
		bool ScissorEnable{};
		bool MultisampleEnable{};
		int DepthBias{};
		float DepthBiasClamp{};
		float SlopeScaledDepthBias{};

		FRasterState()
		: FillMode(FillMode::Solid),
		  CullMode(CullMode::Front),
		  FrontCounterClockwise(false),
		  DepthClipEnable(true),
		  ScissorEnable(false),
		  MultisampleEnable(false),
		  DepthBias(0),
		  DepthBiasClamp(0),
		  SlopeScaledDepthBias(0) { }
	};
}