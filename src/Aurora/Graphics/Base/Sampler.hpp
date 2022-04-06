#pragma once

#include <cstdint>
#include <memory>

#include "Aurora/Graphics/Color.hpp"
#include "TypeBase.hpp"

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
				MipBias(0),
				Anisotropy(1),
				MinFilter(true),
				MagFilter(true),
				MipFilter(true),
				ShadowCompare(false),
				BorderColor(1, 1, 1, 1)
		{
			WrapMode[0] = WrapMode[1] = WrapMode[2] = EWrapMode::Clamp;
		}

		SamplerDesc(bool minFilter, bool magFilter, bool mipFilter, EWrapMode wrapX, EWrapMode wrapY, EWrapMode wrapZ = EWrapMode::Clamp)
		: MipBias(0), Anisotropy(1), MinFilter(minFilter),MagFilter(magFilter),
		  MipFilter(mipFilter),
		  ShadowCompare(false),
		  BorderColor(1, 1, 1, 1)
		{
			WrapMode[0] = wrapX;
			WrapMode[1] = wrapY;
			WrapMode[2] = wrapZ;
		}
	};

	class ISampler : public TypeBase<ISampler>
	{
	public:
		virtual ~ISampler() = default;
		virtual const SamplerDesc& GetDesc() const noexcept = 0;
	};

	typedef std::shared_ptr<ISampler> Sampler_ptr;
}