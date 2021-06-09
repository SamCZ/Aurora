#pragma once

#include <cstdint>
#include <memory>

#include "Aurora/Core/Color.hpp"

namespace Aurora
{
	enum class EWrapMode : uint8_t
	{
		Clamp = 0,
		Wrap,
		Border
	};

	struct SamplerDesc
	{
		EWrapMode WrapMode[3]{};
		float MipBias, Anisotropy;
		bool MinFilter, MagFilter, MipFilter;
		bool ShadowCompare;
		Color BorderColor;

		SamplerDesc() :
				MinFilter(true),
				MagFilter(true),
				MipFilter(true),
				MipBias(0),
				Anisotropy(1),
				ShadowCompare(false),
				BorderColor(1, 1, 1, 1)
		{
			WrapMode[0] = WrapMode[1] = WrapMode[2] = EWrapMode::Clamp;
		}
	};

	class ISampler
	{
	public:
		virtual const SamplerDesc& GetDesc() = 0;
	};

	typedef std::shared_ptr<ISampler> Sampler_ptr;
}