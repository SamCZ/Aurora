#include "RenderTargetManager.hpp"

#include <Aurora/AuroraEngine.hpp>

namespace Aurora
{

	RenderTargetManager::RenderTargetManager() : m_LastSize(0)
	{

	}

	void RenderTargetManager::AddTarget(const String &name, const TEXTURE_FORMAT &format, const Vector4 &clearColor, bool useAsShaderResource, bool useUav, bool autoResize)
	{
		if(m_Targets.contains(name)) {
			std::cerr << "Target " << name << " already exists !" << std::endl;
			return;
		}

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

		m_Targets[name] = {Texture_ptr_null, textureDesc, true};
	}

	Texture_ptr RenderTargetManager::GetTarget(const String &name)
	{
		if(m_Targets.contains(name)) {
			return m_Targets[name].Texture;
		}

		std::cout << "Target " << name << " not found !" << std::endl;

		return Texture_ptr_null;
	}

	ITextureView *RenderTargetManager::GetTargetView(const String &name, const TEXTURE_VIEW_TYPE& viewType)
	{
		auto target = GetTarget(name);

		if(target != nullptr) {
			return target->GetDefaultView(viewType);
		}

		return nullptr;
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

			TextureDesc textureDesc = info.TextureDesc;
			textureDesc.Width = width;
			textureDesc.Height = height;

			AuroraEngine::RenderDevice->CreateTexture(textureDesc, nullptr, &info.Texture);
		}
	}

	void RenderTargetManager::Resize(const Vector2i& size)
	{
		Resize(size.x, size.y);
	}

	void RenderTargetManager::CreatePack(const String &name, const std::vector<String> &target_names, const String &depth_target_name)
	{
		if(m_Packs.contains(name)) {
			std::cerr << "Target pack " << name << " already exists !" << std::endl;
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
				std::cerr << "Target " << targetName << " not found !" << std::endl;
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

	void RenderTargetPack::Apply(bool clear)
	{
		const float ClearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};

		std::vector<ITextureView*> views(m_Targets.size());
		for(const auto& it : m_Targets) {
			views[it.second.first] = it.second.second->Texture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
		}

		ITextureView* depthView = nullptr;
		if(m_DepthTarget != nullptr) {
			depthView = m_DepthTarget->Texture->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
		}

		AuroraEngine::ImmediateContext->SetRenderTargets(views.size(), views.data(), depthView, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		if(clear) {
			if(depthView != nullptr) {
				AuroraEngine::ImmediateContext->ClearDepthStencil(depthView, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			}

			for(ITextureView* view : views) {
				AuroraEngine::ImmediateContext->ClearRenderTarget(view, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			}
		}
	}

	void RenderTargetPack::Apply(GraphicsPipelineDesc &pipelineDesc)
	{
		pipelineDesc.NumRenderTargets = m_Targets.size();
		for(const auto& it : m_Targets) {
			pipelineDesc.RTVFormats[it.second.first] = it.second.second->Texture->GetDesc().Format;
		}

		if(m_DepthTarget != nullptr) {
			pipelineDesc.DSVFormat = m_DepthTarget->Texture->GetDesc().Format;
		} else {
			pipelineDesc.DSVFormat = TEX_FORMAT_UNKNOWN;
		}
	}
}