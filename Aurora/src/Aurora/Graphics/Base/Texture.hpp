#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "Format.hpp"
#include "TypeBase.hpp"
#include "Aurora/Graphics/Color.hpp"
#include "Aurora/Core/Math.hpp"
#include "Aurora/Resource/ResourceName.hpp"

namespace Aurora
{
	enum TEX_FLAGS : uint8_t
	{
		TEX_FLAG_NONE = 0,
		TEX_FLAG_UAV = 1 << 0
	};

	enum class EDimensionType : uint8_t
	{
		TYPE_2D,
		TYPE_3D,
		TYPE_CubeMap,
		TYPE_2DArray
	};

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
		uint32_t SampleCount;
		GraphicsFormat ImageFormat;
		EUsage Usage;
		EDimensionType DimensionType;

		std::string Name = "Unknown";

		bool IsRenderTarget;
		bool IsUAV;
		bool DisableGPUsSync;

		Color ClearValue;
		bool UseClearValue;

		bool UseAsBindless;

		TextureDesc() :
				Width(0),
				Height(0),
				DepthOrArraySize(0),
				MipLevels(1),
				SampleCount(1),
				ImageFormat(GraphicsFormat::Unknown),
				Usage(EUsage::Default),
				DimensionType(EDimensionType::TYPE_2D),
				Name("Unknown"),
				IsRenderTarget(false),
				IsUAV(false),
				DisableGPUsSync(false),
				ClearValue(0), UseClearValue(false), UseAsBindless(false) { }

		[[nodiscard]] inline Vector2i GetSize() const noexcept { return {Width, Height}; }

		[[nodiscard]] inline uint32_t GetMipLevelCount() const
		{
			return GetMipLevelCount(Width, Height);
		}

		static inline uint32_t GetMipLevelCount(int w, int h)
		{
			return (uint32_t)std::floor(std::log2(glm::min(w, h))) + 1;
		}

		[[nodiscard]] Vector2i GetMipSize(uint32_t mip) const
		{
			uint32_t width = Width;
			uint32_t height = Height;
			while (mip != 0)
			{
				width /= 2;
				height /= 2;
				mip--;
			}

			return { width, height };
		}
	};

	class ITexture : public TypeBase<ITexture>
	{
	public:
		ResourceName m_ResourceName;
		bool EnabledBindSRGB = true;
	public:
		virtual ~ITexture() = default;
		[[nodiscard]] virtual const TextureDesc& GetDesc() const = 0;

		virtual void* GetRawHandle() = 0;

		const ResourceName& GetResourceName() const { return m_ResourceName; }
		void SetResourceName(const ResourceName& resourceName) { m_ResourceName = resourceName; }
	};

	typedef std::shared_ptr<ITexture> Texture_ptr;
	#define Texture_ptr_null nullptr
}