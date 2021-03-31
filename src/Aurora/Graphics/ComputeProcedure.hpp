#pragma once

#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <PipelineState.h>
#include <MapHelper.hpp>
#include <CommonlyUsedStates.h>
#include <Shader.h>

#include "Aurora/Core/Common.hpp"
#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{
	AU_CLASS(ComputeProcedure)
	{
	private:
		struct PipelineStateData
		{
			RefCntAutoPtr<IPipelineState> State;
			RefCntAutoPtr<IShaderResourceBinding> ResourceBinding;
		};

		struct ShaderConstantBuffer
		{
			uint32_t Size;
			RefCntAutoPtr<IBuffer> Buffer;
			BufferDesc Desc;
			std::vector<uint8_t> BufferData;
			std::vector<ShaderVariable> Variables;
			bool NeedsUpdate;
		};

		struct ShaderTextureDef
		{
			std::vector<SHADER_TYPE> ShaderTypes;
			RefCntAutoPtr<ITexture> TextureRef;
			ITextureView* TextureView;
			bool NeedsUpdate;
		};
	private:
		String m_Name;
		RefCntAutoPtr<IShader> m_Shader;

		RefCntAutoPtr<IPipelineState> m_CurrentPipelineState;
		RefCntAutoPtr<IShaderResourceBinding> m_CurrentResourceBinding;
		std::map<uint32_t, PipelineStateData> m_PipelineStates;
		uint32_t m_CurrentPipelineStateHash;
		ComputePipelineStateCreateInfo m_CPSCInfo;

		std::map<String, ShaderConstantBuffer> m_ShaderConstantBuffers;
		std::vector<ShaderResourceVariableDesc> m_ShaderResourceVariables;
		std::vector<ImmutableSamplerDesc> m_ShaderResourceSamplers;
		std::map<String, ShaderTextureDef> m_ShaderTextures;
	public:
		ComputeProcedure(String name, const RefCntAutoPtr<IShader>& computeShader, const std::map<String, String>& macros = {});
		ComputeProcedure(const String& name, const Path& shaderPath, const std::map<String, String>& macros = {});
		void Dispatch(uint32_t ThreadGroupCountX = 1, uint32_t ThreadGroupCountY = 1, uint32_t ThreadGroupCountZ = 1);

	private:
		void Validate();
		static uint32_t GetCreateInfoHash(ComputePipelineStateCreateInfo info);

	public:
		template <typename T>
		inline bool SetVariable(const String& name, T data)
		{
			void* rawData = (void*)(&data);
			size_t size = sizeof(data);

			for(auto& it : m_ShaderConstantBuffers) {
				const String &buffer_name = it.first;
				ShaderConstantBuffer &buffer = it.second;

				for(auto& var : buffer.Variables) {
					if(var.Name == name) {
						if(var.Size == size) {
							memcpy(buffer.BufferData.data() + var.Offset, rawData, size);
							buffer.NeedsUpdate = true;
							return true;
						} else {
							AU_THROW_ERROR("Size is not exact !");
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
				const String &buffer_name = it.first;
				ShaderConstantBuffer &buffer = it.second;

				for(auto& var : buffer.Variables) {
					if(var.Name == name) {
						memcpy(outData, buffer.BufferData.data(), size);
						return true;
					}
				}
			}

			return false;
		}

		inline ComputePipelineStateCreateInfo & GetComputePipelineStateCreateInfo()
		{
			return m_CPSCInfo;
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

		inline void SetTexture(const String& name, const RefCntAutoPtr<ITexture>& texture)
		{
			if(texture == nullptr) {
				std::cerr << "Null texture " << name << " in " << m_Name << std::endl;
				return;
			}

			// This is just internal! Don't do this!!!
			auto& tex = const_cast<RefCntAutoPtr<ITexture>&>(texture);

			for (auto& var : m_ShaderTextures) {
				if(var.first == name) {
					var.second.TextureRef = texture;
					var.second.TextureView = tex->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS);
					var.second.NeedsUpdate = true;
				}
			}
		}
	};
}
