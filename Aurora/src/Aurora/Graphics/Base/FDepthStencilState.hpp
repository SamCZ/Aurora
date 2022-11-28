#pragma once

#include <cstdint>

namespace Aurora
{
	enum class EDepthWriteMask : uint8_t
	{
		Zero = 0,
		All = 1
	};

	enum class EStencilOp : uint8_t
	{
		Keep = 1,
		Zero = 2,
		Replace = 3,
		IncrSat = 4,
		DecrSat = 5,
		Invert = 6,
		Increment = 7,
		Decrement = 8
	};

	enum class EComparisonFunc : uint8_t
	{
		Never = 1,
		Less = 2,
		Equal = 3,
		LessEqual = 4,
		Greater = 5,
		NotEqual = 6,
		GreaterEqual = 7,
		Always = 8
	};

	struct FDepthStencilState
	{
		struct StencilOpDesc
		{
			EStencilOp StencilFailOp;
			EStencilOp StencilDepthFailOp;
			EStencilOp StencilPassOp;
			EComparisonFunc StencilFunc;
		};

		bool            DepthEnable;
		EDepthWriteMask DepthWriteMask;
		EComparisonFunc DepthFunc;
		bool            StencilEnable;
		uint8_t         StencilReadMask;
		uint8_t         StencilWriteMask;
		uint8_t         StencilRefValue;
		uint8_t         Padding;
		StencilOpDesc   FrontFace{};
		StencilOpDesc   BackFace{};

		FDepthStencilState() :
				DepthEnable(true),
				DepthWriteMask(EDepthWriteMask::All),
				DepthFunc(EComparisonFunc::Less),
				StencilEnable(false),
				StencilReadMask(0xff),
				StencilWriteMask(0xff),
				StencilRefValue(0),
				Padding(0)
		{
			StencilOpDesc stencilOpDesc = {};
			stencilOpDesc.StencilFailOp = EStencilOp::Keep;
			stencilOpDesc.StencilDepthFailOp = EStencilOp::Keep;
			stencilOpDesc.StencilPassOp = EStencilOp::Keep;
			stencilOpDesc.StencilFunc = EComparisonFunc::Always;
			FrontFace = stencilOpDesc;
			BackFace = stencilOpDesc;
		}
	};
}