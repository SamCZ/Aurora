#include "Material.hpp"

#include <utility>
#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{

	Material::Material(String name, const Path &shaderPath, const ShaderMacros &macros) : m_Name(std::move(name))
	{
		m_Shader = ASM->LoadShaderFolder(shaderPath, macros);

		LoadShaderResources(m_Shader);
	}

	Material::Material(String name, Shader_ptr shader) : m_Name(std::move(name)), m_Shader(std::move(shader))
	{
		LoadShaderResources(m_Shader);
	}

	void Material::LoadShaderResources(const Shader_ptr &shader)
	{
		for(const auto& res : shader->GetResources(ShaderResourceType::ConstantBuffer)) {
			ShaderConstantBuffer constantBuffer = {};
			constantBuffer.Name = res.Name;
			constantBuffer.BufferData.resize(res.Size);
			constantBuffer.Buffer = RD->CreateBuffer(BufferDesc(res.Name, res.Size, 0, EBufferType::UniformBuffer, EBufferUsage::DynamicDraw));
			constantBuffer.Variables = res.Variables;
			constantBuffer.NeedsUpdate = false;
			constantBuffer.Size = res.Size;
			constantBuffer.ShaderType = res.ShadersIn;

			m_ShaderConstantBuffers.push_back(constantBuffer);
		}

		for(const auto& res : shader->GetResources(ShaderResourceType::TextureSRV)) {
			ShaderTextureDef shaderTextureDef = {};
			shaderTextureDef.Name = res.Name;
			shaderTextureDef.TextureRef = nullptr;
			shaderTextureDef.NeedsUpdate = true;
			m_ShaderTextures.emplace_back(shaderTextureDef);
		}

		for(const auto& res : shader->GetResources(ShaderResourceType::Sampler)) {
			m_ShaderSamplers.emplace_back(res.Name, nullptr);
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
	}

	bool Material::SetVariable(const String &name, void* data, size_t size)
	{
		for(auto& cb : m_ShaderConstantBuffers) {
			for(const auto& cb_var : cb.Variables) {
				if(cb_var.Name == name) {
					assert(cb_var.Size == size);
					memcpy(cb.BufferData.data() + cb_var.Offset, data, std::min(cb_var.Size, size));
					cb.NeedsUpdate = true;
					return true;
				}
			}
		}

		//AU_LOG_WARNING("Variable ", name, " in material ", m_Name, " was not found !");

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

		//AU_LOG_WARNING("Texture ", name, " in material ", m_Name, " was not found !");
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
}