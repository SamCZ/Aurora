#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "Format.hpp"
#include "TypeBase.hpp"
#include "Aurora/Core/Color.hpp"

namespace Aurora
{
	struct TextureDesc
	{
		enum class EUsage : uint8_t
		{
			Default = 0,
			Immutable,
			Dynamic
		};

		uint32_t Width;
		uint32_t Height;
		uint32_t DepthOrArraySize;
		uint32_t MipLevels;
		uint32_t SampleCount, SampleQuality;
		GraphicsFormat ImageFormat;
		EUsage Usage;
		std::string DebugName;

		bool IsArray; //3D or array if .z != 0?
		bool IsCubeMap;
		bool IsRenderTarget;
		bool IsUAV;
		bool IsCPUWritable;
		bool DisableGPUsSync;

		Color ClearValue;
		bool UseClearValue;

		TextureDesc() :
				ImageFormat(GraphicsFormat::Unknown),
				Width(0),
				Height(0),
				DepthOrArraySize(0),
				MipLevels(1),
				Usage(EUsage::Default),
				SampleCount(1),
				SampleQuality(0),
				DebugName(),
				IsCPUWritable(false),
				IsUAV(false),
				IsRenderTarget(false),
				IsArray(false),
				IsCubeMap(false),
				DisableGPUsSync(false),
				UseClearValue(false), ClearValue(0) { }
	};

	class ITexture : public TypeBase<ITexture>
	{
	public:
		virtual ~ITexture() = default;
		[[nodiscard]] virtual const TextureDesc& GetDesc() const = 0;
	};

	typedef std::shared_ptr<ITexture> Texture_ptr;
	#define Texture_ptr_null nullptr
}