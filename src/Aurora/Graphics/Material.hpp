#pragma once

#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <PipelineState.h>
#include <MapHelper.hpp>

#include "ShaderCollection.hpp"
#include "Aurora/Core/Common.hpp"

#include "Aurora/AuroraEngine.hpp"

using namespace Diligent;

namespace Aurora
{
	AU_CLASS(Material)
	{
	private:
		struct PipelineStateData
		{
			RefCntAutoPtr<IPipelineState> State;
			RefCntAutoPtr<IShaderResourceBinding> ResourceBinding;
		};

		struct ShaderConstantBuffer
		{
			std::vector<SHADER_TYPE> ShaderTypes;
			uint32_t Size;
			RefCntAutoPtr<IBuffer> Buffer;
			BufferDesc Desc;
		};

		struct ShaderTextureDef
		{
			std::vector<SHADER_TYPE> ShaderTypes;
			ITextureView* TextureView;
			bool NeedsUpdate;
		};
	private:
		String m_Name;
		ShaderCollection_ptr m_ShaderCollection;

		RefCntAutoPtr<IPipelineState> m_CurrentPipelineState;
		RefCntAutoPtr<IShaderResourceBinding> m_CurrentResourceBinding;
		std::map<uint32_t, PipelineStateData> m_PipelineStates;
		uint32_t m_CurrentPipelineStateHash;
		GraphicsPipelineStateCreateInfo m_PSOCreateInfo;

		std::map<String, ShaderConstantBuffer> m_ShaderConstantBuffers;

		std::vector<ShaderResourceVariableDesc> m_ShaderResourceVariables;
		std::vector<ImmutableSamplerDesc> m_ShaderResourceSamplers;

		std::map<String, ShaderTextureDef> m_ShaderTextures;
	public:
		explicit Material(const String& name, ShaderCollection_ptr shaderCollection);
		~Material() = default;
	private:
		void InitShaderResources(const RefCntAutoPtr<IShader>& shader);
		void AssignShaders();
	public:
		template <typename DataType, bool KeepStrongReferences = false>
		bool GetConstantBuffer(const String& name, MapHelper<DataType, KeepStrongReferences>& map)
		{
			auto iter = m_ShaderConstantBuffers.find(name);
			if(iter == m_ShaderConstantBuffers.end()) {
				return false;
			}

			map = MapHelper<DataType, KeepStrongReferences>(AuroraEngine::ImmediateContext, iter->second.Buffer, MAP_WRITE, MAP_FLAG_DISCARD);

			return true;
		}

		void ValidateGraphicsPipelineState();

		void ApplyPipeline();
		void CommitShaderResources();

		GraphicsPipelineDesc& GetPipelineDesc();

		inline GraphicsPipelineStateCreateInfo& GetGraphicsPipelineStateCreateInfo()
		{
			return m_PSOCreateInfo;
		}

		[[nodiscard]] inline uint32_t GetHash() const noexcept
		{
			return m_CurrentPipelineStateHash;
		}

		inline RefCntAutoPtr<IPipelineState>& GetCurrentPipelineState()
		{
			return m_CurrentPipelineState;
		}

		inline RefCntAutoPtr<IShaderResourceBinding>& GetCurrentResourceBinding()
		{
			return m_CurrentResourceBinding;
		}

	public:
		inline void SetSampler(const String& textureName, const SamplerDesc& sampler)
		{
			for (auto& m_ShaderResourceSampler : m_ShaderResourceSamplers) {
				if(textureName == m_ShaderResourceSampler.SamplerOrTextureName) {
					m_ShaderResourceSampler.Desc = sampler;
				}
			}
		}

		inline void SetTexture(const String& name, RefCntAutoPtr<ITexture>& texture)
		{
			if(texture == nullptr) {
				std::cerr << "Null texture " << name << " in " << m_Name << std::endl;
				return;
			}

			for (auto& var : m_ShaderTextures) {
				if(var.first == name) {
					var.second.TextureView = texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
					var.second.NeedsUpdate = true;
				}
			}
		}
	public:
		inline void SetFillMode(const FILL_MODE& fillMode) noexcept
		{
			m_PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FillMode = fillMode;
		}

		inline void SetCullMode(const CULL_MODE& cullMode) noexcept
		{
			m_PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = cullMode;
		}

		inline void SetBlendStateForRenderTarget(int index, const RenderTargetBlendDesc& blendDesc)
		{
			m_PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[index] = blendDesc;
		}

		inline void SetBlendState(const RenderTargetBlendDesc& blendDesc)
		{
			SetBlendStateForRenderTarget(0, blendDesc);
		}

		inline void SetIndependentBlend(bool flag) noexcept
		{
			m_PSOCreateInfo.GraphicsPipeline.BlendDesc.IndependentBlendEnable = flag;
		}

		inline void SetPrimitiveTopology(const PRIMITIVE_TOPOLOGY& primitiveTopology) noexcept
		{
			m_PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = primitiveTopology;
		}

		inline void SetDepthEnable(bool enabled) noexcept
		{
			m_PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = enabled;
		}

		inline void SetDepthWriteEnable(bool enabled) noexcept
		{
			m_PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = enabled;
		}

		inline void SetDepthFunc(const COMPARISON_FUNCTION& comparisonFunction) noexcept
		{
			m_PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc = comparisonFunction;
		}
	private:
		static uint32_t GetGraphicsPipelineStateCreateInfoHash(Diligent::GraphicsPipelineStateCreateInfo& gpsci);
	};
}
