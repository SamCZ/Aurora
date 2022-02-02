#pragma once

#include <set>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Tools/robin_hood.h"
#include "Aurora/Engine.hpp"
#include "Base/Format.hpp"
#include "Base/Texture.hpp"
#include "Base/ShaderBase.hpp"
#include "Base/Sampler.hpp"
#include "Base/Buffer.hpp"
#include "BufferCache.hpp"

namespace Aurora
{
	class IRenderDevice;

	class RenderManager;

	namespace Samplers
	{
		AU_API extern Sampler_ptr ClampClampLinearLinear;
		AU_API extern Sampler_ptr WrapWrapLinearLinear;
		AU_API extern Sampler_ptr WrapWrapNearNearestFarLinear;

		AU_API extern Sampler_ptr ClampClampNearestNearest;
		AU_API extern Sampler_ptr ClampClampClampLinearLinearLinear;
		AU_API extern Sampler_ptr WrapWrapNearestNearest;
		AU_API extern Sampler_ptr ClampClampNearNearestFarLinear;
	}

	struct RTCacheSort
	{
		uint Width;
		uint Height;
		GraphicsFormat Format;
		EDimensionType DimensionType;
		uint MipLevels;
		uint DepthOrArraySize;
		TextureDesc::EUsage Usage;
		bool UnorderedAccessView;

		uint Handle;

		[[nodiscard]] int compareNH(const RTCacheSort &other) const
		{
			if (Width < other.Width) return -1;
			if (Width > other.Width) return 1;
			if (Height < other.Height) return -1;
			if (Height > other.Height) return 1;

			if ((uint8_t) Format < (uint8_t) other.Format) return -1;
			if ((uint8_t) Format > (uint8_t) other.Format) return 1;

			if (DimensionType < other.DimensionType) return -1;
			if (DimensionType > other.DimensionType) return 1;

			if (MipLevels < other.MipLevels) return -1;
			if (MipLevels > other.MipLevels) return 1;

			if (DepthOrArraySize < other.DepthOrArraySize) return -1;
			if (DepthOrArraySize > other.DepthOrArraySize) return 1;

			if ((uint8_t) Usage < (uint8_t) other.Usage) return -1;
			if ((uint8_t) Usage > (uint8_t) other.Usage) return 1;

			if (UnorderedAccessView < other.UnorderedAccessView) return -1;
			if (UnorderedAccessView > other.UnorderedAccessView) return 1;

			return 0;
		}

		[[nodiscard]] int compare(const RTCacheSort &other) const
		{
			int comp = compareNH(other);
			if (comp) return comp;
			if (Handle < other.Handle) return -1;
			if (Handle > other.Handle) return 1;
			return 0;
		}

		bool operator<(const RTCacheSort &other) const
		{ return compare(other) < 0; }

		bool operator>(const RTCacheSort &other) const
		{ return compare(other) > 0; }

		bool operator==(const RTCacheSort &other) const
		{ return compare(other) == 0; }
	};

	class AU_API TemporalRenderTarget
	{
		friend class RenderManager;

	private:
		Texture_ptr m_Texture;
		RenderManager *m_Manager;
		int m_Index = -1;
	public:
		operator const Texture_ptr &()
		{
			return m_Texture;
		}

		const Texture_ptr &operator->() const
		{
			return m_Texture;
		}

		void Free();
	};

	struct TemporalRenderTargetStorage
	{
		String Name;
		RTCacheSort Cache;
		Texture_ptr Texture;
		double LastUseTime;
	};

	class AU_API RenderManager
	{
		friend class TemporalRenderTarget;

	private:
		IRenderDevice *m_RenderDevice;

		Shader_ptr m_BlitShader;
		std::vector<TemporalRenderTargetStorage> m_TemporalRenderTargets;

		BufferCache m_UniformBufferCache;
	public:
		explicit RenderManager(IRenderDevice *renderDevice);

		~RenderManager();

		TemporalRenderTarget CreateTemporalRenderTarget(const String &name, uint width, uint height, GraphicsFormat format, EDimensionType dimensionType = EDimensionType::TYPE_2D, uint mipLevels = 1,
														uint depthOrArraySize = 0, TextureDesc::EUsage usage = TextureDesc::EUsage::Default, bool uav = false);

		TemporalRenderTarget CreateTemporalRenderTarget(const String &name, const Vector2i &size, GraphicsFormat format, EDimensionType dimensionType = EDimensionType::TYPE_2D, uint mipLevels = 1,
														uint depthOrArraySize = 0, TextureDesc::EUsage usage = TextureDesc::EUsage::Default, bool uav = false)
		{
			return std::move(CreateTemporalRenderTarget(name, size.x, size.y, format, dimensionType, mipLevels, depthOrArraySize, usage, uav));
		}

		Texture_ptr
		CreateRenderTarget(const String &name, uint width, uint height, GraphicsFormat format, EDimensionType dimensionType = EDimensionType::TYPE_2D, uint mipLevels = 1, uint depthOrArraySize = 0,
						   TextureDesc::EUsage usage = TextureDesc::EUsage::Default, bool uav = false);

		Texture_ptr
		CreateRenderTarget(const String &name, const Vector2i &size, GraphicsFormat format, EDimensionType dimensionType = EDimensionType::TYPE_2D, uint mipLevels = 1, uint depthOrArraySize = 0,
						   TextureDesc::EUsage usage = TextureDesc::EUsage::Default, bool uav = false)
		{
			return std::move(CreateRenderTarget(name, size.x, size.y, format, dimensionType, mipLevels, depthOrArraySize, usage, uav));
		}

		Texture_ptr CreateTextureArray(const std::vector<Path>& textures, bool srgba);
		Texture_ptr CreateCubeMap(const std::array<Path, 6>& textures, bool srgba);

		void Blit(const Texture_ptr &src, const Texture_ptr &dest);

		void Blit(const Texture_ptr &src)
		{
			Blit(src, nullptr);
		}

		// Not used for now
		BufferCache &GetUniformBufferCache()
		{
			return m_UniformBufferCache;
		}

		void EndFrame();
	};

#define BEGIN_UB(type, name) \
{ \
    VBufferCacheIndex cacheIndex; \
    auto* name = GetEngine()->GetRenderManager()->GetUniformBufferCache().GetOrMap<type>(sizeof(type) * 1, cacheIndex);

#define END_UB(bufferName) \
    GetEngine()->GetRenderManager()->GetUniformBufferCache().Unmap(cacheIndex); \
    drawState.BindUniformBuffer(#bufferName, cacheIndex.Buffer, cacheIndex.Offset, cacheIndex.Size);}

#define END_CUB(bufferName) \
    m_RenderManager->GetUniformBufferCache().Unmap(cacheIndex); \
    dispatchState.BindUniformBuffer(#bufferName, cacheIndex.Buffer, cacheIndex.Offset, cacheIndex.Size);}

#define BEGIN_UBW(type, name) \
    { type l_BufferData = {}; type* name = &l_BufferData; auto l_BufferDataSize = sizeof(type);

#define END_UBW(state, buffer, uniformBufferName) \
     GetEngine()->GetRenderDevice()->WriteBuffer(buffer, &l_BufferData, l_BufferDataSize, 0); state.BindUniformBuffer(uniformBufferName, buffer); }
}