#include "Material.hpp"

#include <utility>
#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{
	std::vector<Material*> Material::m_LoadedMaterials;

	Material::Material(String name, const Path &shaderPath, const ShaderMacros &macros) : m_Name(std::move(name)), m_QueueBucket(QueueBucket::Opaque), m_Enabled(true), m_ShaderConstantBuffers()
	{
		m_Shader = ASM->LoadShaderFolder(shaderPath, macros);

		LoadShaderResources(m_Shader);

		m_LoadedMaterials.push_back(this);
	}

	Material::Material(String name, Shader_ptr shader) : m_Name(std::move(name)), m_Shader(std::move(shader)), m_QueueBucket(QueueBucket::Opaque), m_Enabled(true), m_ShaderConstantBuffers()
	{
		LoadShaderResources(m_Shader);

		m_LoadedMaterials.push_back(this);
	}

	Material::~Material()
	{
		auto it = std::find(m_LoadedMaterials.begin(), m_LoadedMaterials.end(), this);

		if(it == m_LoadedMaterials.end()) return;

		m_LoadedMaterials.erase(it);
	}

	void Material::LoadShaderResources(const Shader_ptr &shader)
	{
		m_ShaderConstantBuffers.clear();
		m_ShaderTextures.clear();
		m_ShaderSamplers.clear();

		for(const auto& res : shader->GetResources(ShaderResourceType::ConstantBuffer)) {
			ShaderConstantBuffer constantBuffer = {};
			constantBuffer.Name = res.Name;
			constantBuffer.BufferData = {};
			constantBuffer.BufferData.resize(res.Size);
			constantBuffer.Desc = BufferDesc(res.Name, res.Size, 0, EBufferType::UniformBuffer, EBufferUsage::DynamicDraw);
			constantBuffer.Buffer = RD->CreateBuffer(constantBuffer.Desc);
			constantBuffer.Variables = res.Variables;
			constantBuffer.NeedsUpdate = false;
			constantBuffer.Size = res.Size;
			constantBuffer.ShaderType = res.ShadersIn;

			std::fill(constantBuffer.BufferData.begin(), constantBuffer.BufferData.end(), 0u);

			m_ShaderConstantBuffers.emplace_back(constantBuffer);
		}

		static Sampler_ptr baseSampler = RD->CreateSampler(SamplerDesc());

		for(const auto& res : shader->GetResources(ShaderResourceType::TextureSRV)) {
			ShaderTextureDef shaderTextureDef = {};
			shaderTextureDef.Name = res.Name;
			shaderTextureDef.TextureRef = nullptr;
			shaderTextureDef.NeedsUpdate = true;

			m_ShaderTextures.emplace_back(shaderTextureDef);
			m_ShaderSamplers.push_back({res.Name, baseSampler});
		}

		/*for(const auto& res : shader->GetResources(ShaderResourceType::Sampler)) {
			m_ShaderSamplers.emplace_back(res.Name, baseSampler);
		}*/
	}

	void Material::ReloadShader()
	{
		const ShaderProgramDesc& oldDesc = m_Shader->GetDesc();

		Shader_ptr newShader;
		if(m_Shader->GetDesc().GetShaderDescriptions().size() > 1)
		{
			newShader = ASM->LoadShaderFolder(oldDesc.GetName(), oldDesc.GetShaderDescriptions().begin()->second.Macros);
		}
		else
		{
			newShader = ASM->LoadComputeShader(oldDesc.GetName(), oldDesc.GetShaderDescriptions().begin()->second.Macros);
		}

		if(newShader == nullptr)
		{
			return;
		}

		m_Shader = newShader;

		ConstantBufferList constantBufferListCopy = m_ShaderConstantBuffers;
		TextureDefList textureDefListCopy = m_ShaderTextures;
		SamplerList samplerListCopy = m_ShaderSamplers;

		LoadShaderResources(m_Shader);

		for (const ShaderConstantBuffer& oldConstantBuffer : constantBufferListCopy)
		{
			for(const ShaderVariable& oldShaderVariable : oldConstantBuffer.Variables)
			{
				auto var = FindVariable(oldShaderVariable.Name);

				if(var.has_value() && var->second.Size == oldShaderVariable.Size && oldConstantBuffer.ShaderType == var->first.ShaderType)
				{
					const void* oldDataPointer = oldConstantBuffer.BufferData.data() + oldShaderVariable.Offset;
					//const float* testData = reinterpret_cast<const float*>(oldDataPointer);
					//AU_LOG_INFO(PointerToString(oldDataPointer), " - ", oldShaderVariable.Offset, " / ", oldConstantBuffer.BufferData.size(), " = ", *testData, ", ", var->second.Size)
					SetVariable(*var, oldDataPointer, var->second.Size);
				}
			}
		}

		for (const ShaderTextureDef& item : textureDefListCopy)
		{
			SetTexture(item.Name, item.TextureRef);
		}

		for (const auto &item : samplerListCopy)
		{
			SetSampler(item.first, item.second);
		}
	}

	void Material::UpdateResources()
	{
		for(auto& cb : m_ShaderConstantBuffers) {
			if(cb.NeedsUpdate) {
				cb.NeedsUpdate = false;

				RD->WriteBuffer(cb.Buffer, cb.BufferData.data(), cb.Size);
			}
		}
	}

	void Material::Apply(DrawCallState& state)
	{
		UpdateResources();
		state.Shader = m_Shader;

		state.ResetResources();

		for(auto& cb : m_ShaderConstantBuffers) {
			state.BindUniformBuffer(cb.Name, cb.Buffer);
		}

		for(auto& tb : m_ShaderTextures) {
			if(tb.TextureRef == nullptr) {
				continue;
			}

			state.BindTexture(tb.Name, tb.TextureRef, false, TextureBinding::EAccess::Read);
		}

		for(auto& sb : m_ShaderSamplers) {
			if(sb.second == nullptr) {
				continue;
			}

			state.BindSampler(sb.first, sb.second);
		}

		state.PrimitiveType = PrimitiveType();
		state.RasterState = RasterState();
		state.DepthStencilState = DepthStencilState();
		// TODO: Blending
	}

	void Material::Apply(DispatchState& state)
	{
		UpdateResources();
		state.Shader = m_Shader;

		state.ResetResources();

		for(auto& cb : m_ShaderConstantBuffers) {
			state.BindUniformBuffer(cb.Name, cb.Buffer);
		}

		for(auto& tb : m_ShaderTextures) {
			if(tb.TextureRef == nullptr) {
				continue;
			}

			state.BindTexture(tb.Name, tb.TextureRef, false, TextureBinding::EAccess::Read);
		}
	}

	bool Material::SetVariable(const String &name, void* data, size_t size)
	{
		auto var = FindVariable(name);

		if(var.has_value())
		{
			SetVariable(*var, data, size);
			return true;
		}

		AU_LOG_WARNING("Variable ", name, " in material ", m_Name, " was not found !");

		return false;
	}

	void Material::SetTexture(const String &name, const Texture_ptr &texture)
	{
		for(auto& tb : m_ShaderTextures) {
			if(tb.Name == name) {
				tb.TextureRef = texture;
				tb.NeedsUpdate = true;
				return;
			}
		}

		AU_LOG_WARNING("Texture ", name, " in material ", m_Name, " was not found !");
	}

	void Material::SetSampler(const String &name, const Sampler_ptr &sampler)
	{
		for(auto& sb : m_ShaderSamplers) {
			if(sb.first == name) {
				sb.second = sampler;
				return;
			}
		}

		//AU_LOG_WARNING("Sampler ", name, " in material ", m_Name, " was not found !");
	}

	std::optional<Material::ShaderVarResult> Material::FindVariable(const String &varName)
	{
		for(Material::ShaderConstantBuffer& cb : m_ShaderConstantBuffers) {
			for(const ShaderVariable& cb_var : cb.Variables) {
				if(cb_var.Name == varName) {
					return Material::ShaderVarResult(cb, cb_var);
				}
			}
		}

		return std::nullopt;
	}

	void Material::SetVariable(const ShaderVarResult& shaderVarResult, const void* data, size_t size)
	{
		Material::ShaderConstantBuffer& cb = std::get<0>(shaderVarResult);
		const ShaderVariable& cv = std::get<1>(shaderVarResult);

		assert(cv.Size == size);
		//std::memcpy(cb.BufferData.data() + cv.Offset, data, size);

		uint8_t* dest = cb.BufferData.data() + cv.Offset;
		const auto* src = reinterpret_cast<const uint8_t*>(data);

		for (int i = 0; i < size; ++i)
		{
			*dest = src[i];
			dest++;
		}

		cb.NeedsUpdate = true;
	}
}