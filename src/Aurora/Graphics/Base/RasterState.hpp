#pragma once

namespace Aurora
{
	enum class EFillMode : unsigned char
	{
		Solid,
		Line
	};

	enum class ECullMode : unsigned char
	{
		Back,
		Front,
		None
	};

	struct FRasterState
	{
		EFillMode    FillMode;
		ECullMode    CullMode;
		bool FrontCounterClockwise{};
		bool DepthClipEnable;
		bool ScissorEnable{};
		bool MultisampleEnable{};
		int DepthBias{};
		float DepthBiasClamp{};
		float SlopeScaledDepthBias{};

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