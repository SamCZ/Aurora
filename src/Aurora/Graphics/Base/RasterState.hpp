#pragma once

namespace Aurora
{
	struct RasterState
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

		FillMode    fillMode;
		CullMode    cullMode;
		bool frontCounterClockwise{};
		bool depthClipEnable;
		bool scissorEnable{};
		bool multisampleEnable{};
		bool antialiasedLineEnable{};
		int depthBias{};
		float depthBiasClamp{};
		float slopeScaledDepthBias{};

		// Extended rasterizer state supported by Maxwell
		// In D3D11, use NvAPI_D3D11_CreateRasterizerState to create such rasterizer state.
		char forcedSampleCount{};
		bool programmableSamplePositionsEnable{};
		bool conservativeRasterEnable{};
		char samplePositionsX[16]{};
		char samplePositionsY[16]{};

		RasterState() : fillMode(FillMode::Solid), cullMode(CullMode::Back)
		{
			depthClipEnable = true;
		}
	};
}