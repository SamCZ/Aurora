#pragma once

#include "IGraphicsPipeline.hpp"
#include "IRenderDevice.hpp"

#include "ShaderCollection.hpp"
#include "Aurora/Core/Common.hpp"

#include "Aurora/Assets/Resources/ShaderResourceObject.hpp"

namespace Aurora
{
	struct ShaderObject
	{
		ShaderType Type;
		ShaderResourceObject_ptr ResourceObject;
		Shader_ptr Shader;
		int ResourceEventId;
	};

	struct ImmutableSamplerDesc
	{

	};

	typedef std::map<ShaderType, ShaderObject> ShaderList;

	AU_CLASS(Material) : public IGraphicsPipeline
	{
	private:
		struct ShaderConstantBuffer
		{
			String Name;
			ShaderType ShaderType;
			uint32_t Size;
			BufferDesc Desc;
			bool NeedsUpdate;

			BufferHandle Buffer;
			std::vector<uint8_t> BufferData;
			std::vector<ShaderVariable> Variables;
		};

		struct ShaderTextureDef
		{
			String Name;
			ShaderType ShaderType;
			TextureHandle TextureRef;
			bool NeedsUpdate;
		};

		typedef std::vector<ShaderConstantBuffer> ConstantBufferList;
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
		std::map<ShaderType, ConstantBufferList> m_ShaderConstantBuffers;
		std::map<ShaderType, TextureDefList> m_ShaderTextures;
		std::map<ShaderType, SamplerList> m_ShaderSamplers;
	private:

	public:
		Material(String name, const Path& shaderPath, ShaderMacros_t macros = {});
		Material(String name, const std::vector<ShaderResourceObject_ptr>& shaders, ShaderMacros_t macros = {});
		explicit Material(String name, ShaderMacros_t macros = {});
		~Material() override;

		void SetShader(const ShaderResourceObject_ptr &sharedPtr);
		void RemoveShader(const ShaderType& shaderType);
		ShaderResourceObject_ptr GetShader(const ShaderType& shaderType);
	public:
		[[nodiscard]] inline const String& GetName() const noexcept { return m_Name; }
	private:
		void OnShaderResourceUpdate(ResourceObject* obj);
		void LoadConstantBuffers(ShaderObject &object, ShaderResourceDesc desc, ConstantBufferList& constantBufferStorage, ConstantBufferList& constantBufferListCopy);
		void LoadTextureSRV(ShaderObject &object, ShaderResourceDesc desc, TextureDefList& textureDefList, TextureDefList& textureDefListOld, SamplerList& samplerList, SamplerList& samplerListOld);
		void DeleteConstantBuffers(const ShaderType& type);
		void DeleteTextureSRVs(const ShaderType &type);
	private:
		void ApplyShaderToPSO(Shader_ptr shader, const ShaderType& shaderType);
	public:
		[[nodiscard]] const ShaderList& GetShaders() const noexcept { return m_Shaders; }
		[[nodiscard]] const ShaderMacros_t& GetMacros() const noexcept { return m_Macros; }

	public:
		void ValidateGraphicsPipelineState();
	public:

	public:
		void ApplyPipeline();
		void CommitShaderResources();
	public:
		inline void SetFillMode(const RasterState::FillMode& fillMode) noexcept { }
		inline void SetCullMode(const RasterState::CullMode& cullMode) noexcept {  }
		inline void SetBlendState(const BlendState& blendDesc) {  }
		inline void SetIndependentBlend(bool flag) noexcept {  }
		inline void SetPrimitiveTopology(const PrimitiveType& primitiveTopology) noexcept {  }
		inline void SetDepthEnable(bool enabled) noexcept {  }
		inline void SetDepthWriteEnable(bool enabled) noexcept {  }
		inline void SetDepthFunc(const DepthStencilState::ComparisonFunc& comparisonFunction) noexcept {  }
	public:
		void SetSampler(const String& textureName, const SamplerDesc& sampler);
		void SetTexture(const String& name, TextureHandle texture);
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
								AU_LOG_ERROR("Size is not exact ! ", var.Name, " - ", var.Size, " - ", var.ArrayStride, " - ", size);
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
								AU_LOG_ERROR("Size is not exact ! ", var.Name, " - ", var.Size, " - ", var.ArrayStride, " - ", size);
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