#pragma once

#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <PipelineState.h>
#include <MapHelper.hpp>
#include <CommonlyUsedStates.h>

#include "IGraphicsPipeline.hpp"

#include "ShaderCollection.hpp"
#include "Aurora/Core/Common.hpp"

#include "Aurora/Assets/Resources/ShaderResourceObject.hpp"

using namespace Diligent;

namespace Aurora
{
	struct ShaderObject
	{
		SHADER_TYPE Type;
		ShaderResourceObject_ptr ResourceObject;
		RefCntAutoPtr<IShader> Shader;
		int ResourceEventId;
	};

	typedef std::map<SHADER_TYPE, ShaderObject> ShaderList;

	struct PSOInfoComparator
	{
		bool operator()(const GraphicsPipelineStateCreateInfo &left, const GraphicsPipelineStateCreateInfo &right) const;
	};

	AU_CLASS(Material) : public IGraphicsPipeline
	{
	private:
		struct PipelineStateData
		{
			RefCntAutoPtr<IPipelineState> State;
			RefCntAutoPtr<IShaderResourceBinding> ResourceBinding;
		};

		struct ShaderConstantBuffer
		{
			String Name;
			SHADER_TYPE ShaderType;
			uint32_t Size;
			BufferDesc Desc;
			bool NeedsUpdate;

			RefCntAutoPtr<IBuffer> Buffer;
			std::vector<uint8_t> BufferData;
			std::vector<ShaderVariable> Variables;
		};

		struct ShaderTextureDef
		{
			String Name;
			SHADER_TYPE ShaderType;
			RefCntAutoPtr<ITexture> TextureRef;
			ITextureView* TextureView;
			bool NeedsUpdate;
		};

		typedef std::vector<ShaderConstantBuffer> ConstantBufferList;
		typedef std::vector<std::pair<ShaderResourceVariableDesc, bool>> ResourceVariableDescList;
		typedef std::vector<ShaderTextureDef> TextureDefList;
		typedef std::vector<std::pair<String, ImmutableSamplerDesc>> SamplerList;
	private:
		static std::vector<Material*> m_CurrentMaterials;
	private:
		String m_Name;
		ShaderMacros_t m_Macros;

		ShaderList m_Shaders;
		std::shared_ptr<ResourceObject::ResourceChangedEvent> m_OnShaderResourceChangeEvent;
	private:
		std::map<SHADER_TYPE, ConstantBufferList> m_ShaderConstantBuffers;
		std::map<SHADER_TYPE, ResourceVariableDescList> m_ShaderResourceVariables;
		std::map<SHADER_TYPE, TextureDefList> m_ShaderTextures;
		std::map<SHADER_TYPE, SamplerList> m_ShaderSamplers;
	private:
		GraphicsPipelineStateCreateInfo m_PSOCreateInfo;
		bool m_NeedsRebuildResourceLayout;
		std::vector<ShaderResourceVariableDesc> m_PSO_ResourceVariableDescList;
		std::vector<ImmutableSamplerDesc> m_PSO_ShaderResourceSamplers;

		GraphicsPipelineStateCreateInfo m_CurrentPipelineStateInfo;
		std::map<GraphicsPipelineStateCreateInfo, PipelineStateData, PSOInfoComparator> m_PipelineStates;

		RefCntAutoPtr<IPipelineState> m_CurrentPipelineState;
		RefCntAutoPtr<IShaderResourceBinding> m_CurrentResourceBinding;
	public:
		Material(String name, const Path& shaderPath, ShaderMacros_t macros = {});
		Material(String name, const std::vector<ShaderResourceObject_ptr>& shaders, ShaderMacros_t macros = {});
		explicit Material(String name, ShaderMacros_t macros = {});
		~Material() override;

		void SetShader(const ShaderResourceObject_ptr &sharedPtr);
		void RemoveShader(const SHADER_TYPE& shaderType);
		ShaderResourceObject_ptr GetShader(const SHADER_TYPE& shaderType);
	public:
		[[nodiscard]] inline const String& GetName() const noexcept { return m_Name; }
	private:
		void OnShaderResourceUpdate(ResourceObject* obj);
		void LoadConstantBuffers(ShaderObject &object, ShaderResourceDesc desc, std::vector<StateTransitionDesc>& barriers, ConstantBufferList& constantBufferStorage, ConstantBufferList& constantBufferListCopy);
		void LoadTextureSRV(ShaderObject &object, ShaderResourceDesc desc, std::vector<StateTransitionDesc>& barriers, TextureDefList& textureDefList, TextureDefList& textureDefListOld, SamplerList& samplerList, SamplerList& samplerListOld);
		void DeleteConstantBuffers(const SHADER_TYPE& type);
		void DeleteTextureSRVs(const SHADER_TYPE &type);
	private:
		void ApplyShaderToPSO(IShader* shader, const SHADER_TYPE& shaderType);
	public:
		[[nodiscard]] const ShaderList& GetShaders() const noexcept { return m_Shaders; }
		[[nodiscard]] const ShaderMacros_t& GetMacros() const noexcept { return m_Macros; }

	public:
		void ValidateGraphicsPipelineState();
	public:
		inline RefCntAutoPtr<IPipelineState>& GetCurrentPipelineState() { return m_CurrentPipelineState; }
		inline RefCntAutoPtr<IShaderResourceBinding>& GetCurrentResourceBinding() { return m_CurrentResourceBinding; }
		inline GraphicsPipelineDesc& GetPipelineDesc() { return m_PSOCreateInfo.GraphicsPipeline; }
	public:
		void ApplyPipeline();
		void CommitShaderResources();
	public:
		inline void SetFillMode(const FILL_MODE& fillMode) noexcept { m_PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FillMode = fillMode; }
		inline void SetCullMode(const CULL_MODE& cullMode) noexcept { m_PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = cullMode; }
		inline void SetBlendStateForRenderTarget(int index, const RenderTargetBlendDesc& blendDesc) { m_PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[index] = blendDesc; }
		inline void SetBlendState(const RenderTargetBlendDesc& blendDesc) { SetBlendStateForRenderTarget(0, blendDesc); }
		inline void SetIndependentBlend(bool flag) noexcept { m_PSOCreateInfo.GraphicsPipeline.BlendDesc.IndependentBlendEnable = flag; }
		inline void SetPrimitiveTopology(const PRIMITIVE_TOPOLOGY& primitiveTopology) noexcept { m_PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = primitiveTopology; }
		inline void SetDepthEnable(bool enabled) noexcept { m_PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = enabled; }
		inline void SetDepthWriteEnable(bool enabled) noexcept { m_PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = enabled; }
		inline void SetDepthFunc(const COMPARISON_FUNCTION& comparisonFunction) noexcept { m_PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc = comparisonFunction; }
	public:
		void SetSampler(const String& textureName, const SamplerDesc& sampler);
		void SetTexture(const String& name, const RefCntAutoPtr<ITexture>& texture);
	public:
		inline static const std::vector<Material*>& GetAllMaterials() noexcept { return m_CurrentMaterials; }
	public:
		template <typename T>
		inline bool SetVariable(const String& name, T data, uint32_t customSize = 0)
		{
			void* rawData = (void*)(&data);
			size_t size = sizeof(data);

			if(customSize > 0) {
				size = customSize;
			}

			for(auto& it : m_ShaderConstantBuffers) {
				for(auto& buffer : it.second) {
					for(auto& var : buffer.Variables) {
						if(var.Name == name) {
							if(var.Size == size) {
								memcpy(buffer.BufferData.data() + var.Offset, rawData, size);
								buffer.NeedsUpdate = true;
							} else {
								AU_THROW_ERROR("Size is not exact ! " << var.Name << " - " << var.Size << " - " << var.ArrayStride << " - " << size);
							}
						}
					}
				}
			}

			return false;
		}

		template <typename T>
		inline bool SetArray(const String& name, T* data, uint32_t customSize = 0)
		{
			size_t size = sizeof(data);

			if(customSize > 0) {
				size = customSize;
			}

			for(auto& it : m_ShaderConstantBuffers) {
				for(auto& buffer : it.second) {
					for(auto& var : buffer.Variables) {
						if(var.Name == name) {
							if(var.Size == size) {
								memcpy(buffer.BufferData.data() + var.Offset, data, size);
								buffer.NeedsUpdate = true;
							} else {
								AU_THROW_ERROR("Size is not exact ! " << var.Name << " - " << var.Size << " - " << var.ArrayStride << " - " << size);
							}
						}
					}
				}
			}

			return false;
		}

		template <typename T>
		inline bool GetVariable(const String& name, T* outData, bool autoSize = true, size_t customSize = 0)
		{
			size_t size = sizeof(T);

			if (!autoSize)
			{
				size = customSize;
			}

			for(auto& it : m_ShaderConstantBuffers) {
				for(auto& buffer : it.second) {
					for(auto& var : buffer.Variables) {
						if(var.Name == name) {
							memcpy(outData, buffer.BufferData.data(), size);
							return true;
						}
					}
				}
			}

			return false;
		}
	};
}