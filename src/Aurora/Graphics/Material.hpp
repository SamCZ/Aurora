#pragma once

#include "Aurora/Core/Common.hpp"
#include "Base/RasterState.hpp"
#include "Base/PrimitiveType.hpp"
#include "Base/BlendState.hpp"
#include "Base/FDepthStencilState.hpp"
#include "Base/Texture.hpp"
#include "Base/ShaderBase.hpp"
#include "Base/Buffer.hpp"
#include "Base/Sampler.hpp"
#include "Base/IRenderDevice.hpp"

namespace Aurora
{
	AU_CLASS(Material)
	{
	private:
		struct ShaderConstantBuffer
		{
			String Name;
			EShaderType ShaderType;
			uint32_t Size;
			BufferDesc Desc;
			bool NeedsUpdate;

			Buffer_ptr Buffer;
			std::vector<uint8_t> BufferData;
			std::vector<ShaderVariable> Variables;
		};

		struct ShaderTextureDef
		{
			String Name;
			EShaderType ShaderType;
			Texture_ptr TextureRef;
			bool NeedsUpdate;
		};

		typedef std::vector<ShaderConstantBuffer> ConstantBufferList;
		typedef std::vector<ShaderTextureDef> TextureDefList;
		typedef std::vector<std::pair<String, Sampler_ptr>> SamplerList;
	private:
		const String m_Name;
		Shader_ptr m_Shader;
	private:
		ConstantBufferList m_ShaderConstantBuffers;
		TextureDefList m_ShaderTextures;
		SamplerList m_ShaderSamplers;
	private:
		FRasterState m_RasterState;
		FDepthStencilState m_DepthStencilState;
		FBlendState m_BlendState;
		EPrimitiveType m_PrimitiveType = EPrimitiveType::TriangleList;
	public:
		Material(String  name, const Path& shaderPath, const ShaderMacros& macros = {});
		Material(String  name, Shader_ptr shader);

		void LoadShaderResources(const Shader_ptr& shader);
		void UpdateResources();
		void Apply(DrawCallState& state);
		void Apply(DispatchState& state);
	public:
		inline void SetFillMode(const FillMode& fillMode) noexcept { m_RasterState.FillMode = fillMode; }
		inline void SetCullMode(const CullMode& cullMode) noexcept { m_RasterState.CullMode = cullMode; }
		inline void SetBlendState(const FBlendState& blendDesc) { m_BlendState = blendDesc; }
		inline void SetIndependentBlend(bool flag) noexcept {  }
		inline void SetPrimitiveTopology(const EPrimitiveType& primitiveTopology) noexcept { m_PrimitiveType = primitiveTopology; }
		inline void SetDepthEnable(bool enabled) noexcept { m_DepthStencilState.DepthEnable = enabled; }
		inline void SetDepthWriteEnable(bool enabled) noexcept { m_DepthStencilState.DepthWriteMask = enabled ? EDepthWriteMask::All : EDepthWriteMask::Zero; }
		inline void SetDepthFunc(const EComparisonFunc& comparisonFunction) noexcept { m_DepthStencilState.DepthFunc = comparisonFunction; }

		inline FRasterState& RasterState() noexcept { return m_RasterState; }
		inline FDepthStencilState& DepthStencilState() noexcept { return m_DepthStencilState; }
		inline FBlendState& BlendState() noexcept { return m_BlendState; }
		inline const EPrimitiveType& PrimitiveType() const noexcept { return m_PrimitiveType; }
	public:
		bool SetVariable(const String& name, void* data, size_t size);

		template<typename Type>
		bool SetVariable(const String& name, Type var)
		{
			void* rawData = (void*)(&var);
			size_t size = sizeof(var);

			return SetVariable(name, rawData, size);
		}

		void SetTexture(const String& name, const Texture_ptr& texture);
		void SetSampler(const String& name, const Sampler_ptr& sampler);
	};
}