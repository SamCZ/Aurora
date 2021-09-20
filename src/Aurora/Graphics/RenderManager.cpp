#include "RenderManager.hpp"

#include "Aurora/Core/Time.hpp"
#include "Aurora/Logger/Logger.hpp"
#include "Base/IRenderDevice.hpp"

#include "InternalShaders/Blit.hpp"

namespace Aurora
{
	RenderManager::RenderManager(IRenderDevice *renderDevice) : m_RenderDevice(renderDevice)
	{
		{
			ShaderProgramDesc desc("Blit");
			desc.AddShader(EShaderType::Vertex, Shaders::INTERNAL_SHADER_BLIT_VS);
			desc.AddShader(EShaderType::Pixel, Shaders::INTERNAL_SHADER_BLIT_PS);
			m_BlitShader = m_RenderDevice->CreateShaderProgram(desc);
		}
	}

	TemporalRenderTarget RenderManager::CreateTemporalRenderTarget(const String &name, uint width, uint height, GraphicsFormat format, EDimensionType dimensionType, uint mipLevels, uint depthOrArraySize, TextureDesc::EUsage usage)
	{
		TemporalRenderTarget rt;

		RTCacheSort cacheSort;
		cacheSort.Width = width;
		cacheSort.Height = height;
		cacheSort.Format = format;
		cacheSort.DimensionType = dimensionType;
		cacheSort.MipLevels = mipLevels;
		cacheSort.DepthOrArraySize = depthOrArraySize;
		cacheSort.Usage = usage;
		cacheSort.Handle = 0;

		auto findRT = [this](const RTCacheSort& sort) -> int
		{
			for (int i = 0; i < m_TemporalRenderTargets.size(); ++i)
			{
				TemporalRenderTargetStorage& storage = m_TemporalRenderTargets[i];
				if(storage.Cache.compare(sort) == 0)
				{
					return i;
				}
			}

			return -1;
		};

		int foundCachedRTIndex = findRT(cacheSort);

		if(foundCachedRTIndex == -1)
		{
			TemporalRenderTargetStorage storage;
			storage.Name = name;
			storage.Cache = cacheSort;
			storage.Cache.Handle = 1;
			storage.Texture = CreateRenderTarget(name, width, height, format, dimensionType, mipLevels, depthOrArraySize, usage);
			storage.LastUseTime = GetTimeInSeconds();

			TemporalRenderTargetStorage& storedTarget = m_TemporalRenderTargets.emplace_back(storage);
			rt.m_Manager = this;
			rt.m_Index = m_TemporalRenderTargets.size() - 1;
			rt.m_Texture = storedTarget.Texture;
		}
		else
		{
			TemporalRenderTargetStorage& storedTarget = m_TemporalRenderTargets[foundCachedRTIndex];
			storedTarget.Name = name;
			storedTarget.Cache.Handle = 1;
			storedTarget.LastUseTime = GetTimeInSeconds();
			rt.m_Manager = this;
			rt.m_Index = foundCachedRTIndex;
			rt.m_Texture = storedTarget.Texture;
		}

		return rt;
	}

	void TemporalRenderTarget::Free()
	{
		assert(m_Index >= 0);
		assert(m_Manager != nullptr);

		m_Manager->m_TemporalRenderTargets[m_Index].Cache.Handle = 0;
		m_Manager = nullptr;
		m_Texture = nullptr;
	}

	Texture_ptr RenderManager::CreateRenderTarget(const String &name, uint width, uint height, GraphicsFormat format, EDimensionType dimensionType, uint mipLevels, uint depthOrArraySize, TextureDesc::EUsage usage)
	{
		TextureDesc textureDesc;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.ImageFormat = format;
		textureDesc.DimensionType = dimensionType;
		textureDesc.MipLevels = mipLevels;
		textureDesc.DepthOrArraySize = depthOrArraySize;
		textureDesc.Usage = usage;
		textureDesc.IsRenderTarget = true;
		textureDesc.Name = name;
		// TODO: UAvs

		return m_RenderDevice->CreateTexture(textureDesc);
	}

	void RenderManager::Blit(const Texture_ptr &src, const Texture_ptr &dest)
	{
		assert(src != nullptr);
		assert(src != dest);

		DrawCallState state;
		state.PrimitiveType = EPrimitiveType::TriangleStrip;
		state.ClearDepthTarget = false;
		state.ClearColorTarget = false;
		state.RasterState.CullMode = ECullMode::None;
		state.DepthStencilState.DepthEnable = false;
		state.Shader = m_BlitShader;

		if(dest != nullptr)
		{
			state.ViewPort = dest->GetDesc().GetSize();
			state.BindTarget(0, dest);
		}
		else
		{
			// Expecting same dest size as src when blitting to back buffer
			state.ViewPort = src->GetDesc().GetSize();
		}

		if(src != nullptr)
		{
			state.BindTexture("Texture", src);
		}

		m_RenderDevice->Draw(state, {DrawArguments(4)});
	}

	void RenderManager::EndFrame()
	{
		double currentTime = GetTimeInSeconds();

		for (size_t i = m_TemporalRenderTargets.size(); i --> 0;)
		{
			const TemporalRenderTargetStorage& storedTarget = m_TemporalRenderTargets[i];

			if(storedTarget.Cache.Handle)
			{
				AU_LOG_FATAL("Render target ", storedTarget.Name, " cannot live outside a frame !");
			}

			// After 10 seconds we destroy old temporal render targets
			if(currentTime - storedTarget.LastUseTime > 10 * 1000)
			{
				AU_LOG_INFO("Deleting TempRT ", storedTarget.Name);
				m_TemporalRenderTargets.erase(m_TemporalRenderTargets.begin() + i);
			}
		}

		if(m_TemporalRenderTargets.size() > 50)
		{
			AU_LOG_WARNING("Temporal render target count exceeded 50, are you sure you are not doing something wrong ?");
		}
	}
}