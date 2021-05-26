#include "RenderTargetManager.hpp"

#include <Aurora/AuroraEngine.hpp>

namespace Aurora
{

	RenderTargetManager::RenderTargetManager() : m_LastSize(0)
	{

	}

	void RenderTargetManager::AddTarget(const String &name, const GraphicsFormat& format, const Vector4 &clearColor, bool useAsShaderResource, bool useUav, bool autoResize)
	{
		if(m_Targets.contains(name)) {
			std::cerr << "Target " << name << " already exists !" << std::endl;
			return;
		}
/*
		TextureDesc textureDesc;
		textureDesc.Name      = name.data();
		textureDesc.Type      = RESOURCE_DIM_TEX_2D;
		textureDesc.Width     = 0;
		textureDesc.Height    = 0;
		textureDesc.MipLevels = 1;
		textureDesc.Format    = format;

		if(format != TEX_FORMAT_D32_FLOAT) {
			// The render target can be bound as a shader resource and as a render target
			textureDesc.BindFlags = BIND_RENDER_TARGET;
			if(useAsShaderResource) {
				textureDesc.BindFlags |= BIND_SHADER_RESOURCE;
			}

			if(useUav) {
				textureDesc.BindFlags |= BIND_UNORDERED_ACCESS;
			}

			// Define optimal clear value
			textureDesc.ClearValue.Format   = textureDesc.Format;
			textureDesc.ClearValue.Color[0] = clearColor.x;
			textureDesc.ClearValue.Color[1] = clearColor.y;
			textureDesc.ClearValue.Color[2] = clearColor.z;
			textureDesc.ClearValue.Color[3] = clearColor.w;
		} else {
			textureDesc.BindFlags = BIND_DEPTH_STENCIL;
			if(useAsShaderResource) {
				textureDesc.BindFlags |= BIND_SHADER_RESOURCE;
			}

			if(useUav) {
				textureDesc.BindFlags |= BIND_UNORDERED_ACCESS;
			}
			// Define optimal clear value
			textureDesc.ClearValue.Format               = textureDesc.Format;
			textureDesc.ClearValue.DepthStencil.Depth   = 1;
			textureDesc.ClearValue.DepthStencil.Stencil = 0;
		}

		m_Targets[name] = {Texture_ptr_null, textureDesc, true};*/
	}

	Texture_ptr RenderTargetManager::GetTarget(const String &name)
	{
		if(m_Targets.contains(name)) {
			return m_Targets[name].Texture;
		}

		AU_LOG_WARNING("Target ", name, " not found !")

		return Texture_ptr_null;
	}

	void RenderTargetManager::Resize(int width, int height)
	{
		if(m_LastSize.x == width && m_LastSize.y == height) {
			return;
		}

		m_LastSize.x = width;
		m_LastSize.y = height;

		for(auto& it : m_Targets) {
			auto& info = it.second;

			/*TextureDesc textureDesc = info.TextureDesc;
			textureDesc.Width = width;
			textureDesc.Height = height;

			AuroraEngine::RenderDevice->CreateTexture(textureDesc, nullptr, &info.Texture);*/
		}
	}

	void RenderTargetManager::Resize(const Vector2i& size)
	{
		Resize(size.x, size.y);
	}

	void RenderTargetManager::CreatePack(const String &name, const std::vector<String> &target_names, const String &depth_target_name)
	{
		if(m_Packs.contains(name)) {
			AU_LOG_WARNING("Target pack ", name, " already exists !")
			return;
		}

		RenderTargetPack_ptr pack = std::make_shared<RenderTargetPack>();
		pack->m_DepthTarget = nullptr;

		if(depth_target_name != "__NONE__" && m_Targets.contains(depth_target_name)) {
			pack->m_DepthTarget = &m_Targets[depth_target_name];
		}

		for (int i = 0; i < target_names.size(); ++i) {
			const String& targetName = target_names[i];

			auto it = m_Targets.find(targetName);

			if(it == m_Targets.end()) {
				AU_LOG_WARNING("Target ", targetName, " not found !")
				return;
			}

			pack->m_Targets[targetName] = std::make_pair(i, &it->second);
		}

		m_Packs[name] = pack;
	}

	RenderTargetPack_ptr RenderTargetManager::GetPack(const String &name)
	{
		auto it = m_Packs.find(name);

		if(it != m_Packs.end()) {
			return it->second;
		}

		return nullptr;
	}

	void RenderTargetManager::Clear()
	{
		/*std::vector<ITextureView*> views(m_Targets.size());
		int i = 0;
		for(auto& it : m_Targets) {
			auto& texture = it.second.Texture;


		}*/
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RenderTargetPack::Apply(DrawCallState &pipelineDesc)
	{
		pipelineDesc.renderState.targetCount = m_Targets.size();
		for(const auto& it : m_Targets) {
			pipelineDesc.renderState.targets[it.second.first] = it.second.second->Texture;
		}

		if(m_DepthTarget != nullptr) {
			pipelineDesc.renderState.depthTarget = m_DepthTarget->Texture;
		} else {
			pipelineDesc.renderState.depthTarget = nullptr;
		}
	}
}