#include "GraphicUtilities.hpp"
#include <Aurora/AuroraEngine.hpp>

#include "Material.hpp"

namespace Aurora
{
	static Material_ptr m_BlitMaterial = nullptr;
	static RefCntAutoPtr<ITexture> m_PlaceholderTexture;

	void GraphicUtilities::Init()
	{
		{
			TextureDesc RTColorDesc;
			RTColorDesc.Name      = "PlaceholderTexture";
			RTColorDesc.Type      = RESOURCE_DIM_TEX_2D;
			RTColorDesc.Width     = 1;
			RTColorDesc.Height    = 1;
			RTColorDesc.MipLevels = 1;
			RTColorDesc.Format    = TEX_FORMAT_RGBA8_UNORM;
			// The render target can be bound as a shader resource and as a render target
			RTColorDesc.BindFlags = BIND_SHADER_RESOURCE;
			// Define optimal clear value
			RTColorDesc.ClearValue.Format   = RTColorDesc.Format;
			RTColorDesc.ClearValue.Color[0] = 1;
			RTColorDesc.ClearValue.Color[1] = 1;
			RTColorDesc.ClearValue.Color[2] = 1;
			RTColorDesc.ClearValue.Color[3] = 1;
			AuroraEngine::RenderDevice->CreateTexture(RTColorDesc, nullptr, &m_PlaceholderTexture);
		}

		m_BlitMaterial = Setup2DMaterial(std::make_shared<Material>("Blit", "Assets/Shaders/Blit"), true);
	}

	void GraphicUtilities::Destroy()
	{
		m_BlitMaterial.reset();
		m_PlaceholderTexture.Release();
	}

	RefCntAutoPtr<ITexture> GraphicUtilities::CreateTextureArray(const std::vector<RefCntAutoPtr<ITexture>>& textures)
	{
		RefCntAutoPtr<ITexture> pTexArray;

		for (int i = 0; i < textures.size(); ++i) {
			RefCntAutoPtr<ITexture> SrcTex = textures[i];
			const auto& TexDesc  = SrcTex->GetDesc();

			if (pTexArray == nullptr)
			{
				//	Create texture array
				auto TexArrDesc      = TexDesc;
				TexArrDesc.ArraySize = textures.size();
				TexArrDesc.Type      = RESOURCE_DIM_TEX_2D_ARRAY;
				TexArrDesc.Usage     = USAGE_DEFAULT;
				TexArrDesc.BindFlags = BIND_SHADER_RESOURCE;
				AuroraEngine::RenderDevice->CreateTexture(TexArrDesc, nullptr, &pTexArray);
			}

			if(pTexArray->GetDesc().Width != TexDesc.Width || pTexArray->GetDesc().Height != TexDesc.Height) {
				std::cout << pTexArray->GetDesc().Width << ":" << pTexArray->GetDesc().Height << std::endl;
				std::cout << TexDesc.Width << ":" << TexDesc.Height << std::endl;
				std::cerr << "Cannot create texture array with different texture sizes ! (" << String(TexDesc.Name) << ")" << std::endl;
				exit(1);
			}

			// Copy current texture into the texture array
			for (Uint32 mip = 0; mip < TexDesc.MipLevels; ++mip)
			{
				CopyTextureAttribs CopyAttribs(SrcTex, RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
											   pTexArray, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
				CopyAttribs.SrcMipLevel = mip;
				CopyAttribs.DstMipLevel = mip;
				CopyAttribs.DstSlice    = i;
				AuroraEngine::ImmediateContext->CopyTexture(CopyAttribs);
			}
		}

		//AuroraEngine::ImmediateContext->GenerateMips(pTexArray->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

		return pTexArray;
	}

	RefCntAutoPtr<ITexture> GraphicUtilities::CreateCubeMap(const std::array<RefCntAutoPtr<ITexture>, 6>& textures)
	{
		RefCntAutoPtr<ITexture> pTexArray;

		for (int i = 0; i < 6; ++i) {
			RefCntAutoPtr<ITexture> SrcTex = textures[i];
			const auto& TexDesc  = SrcTex->GetDesc();

			if (pTexArray == nullptr)
			{
				//	Create texture array
				auto TexArrDesc      = TexDesc;
				TexArrDesc.ArraySize = textures.size();
				TexArrDesc.Type      = RESOURCE_DIM_TEX_CUBE;
				TexArrDesc.Usage     = USAGE_DEFAULT;
				TexArrDesc.BindFlags = BIND_SHADER_RESOURCE;
				AuroraEngine::RenderDevice->CreateTexture(TexArrDesc, nullptr, &pTexArray);
			}

			if(pTexArray->GetDesc().Width != TexDesc.Width || pTexArray->GetDesc().Height != TexDesc.Height) {
				std::cerr << "Cannot create cubeMap with different texture sizes ! (" << TexDesc.Name << ")" << std::endl;
				exit(1);
			}

			// Copy current texture into the texture array
			for (Uint32 mip = 0; mip < TexDesc.MipLevels; ++mip)
			{
				CopyTextureAttribs CopyAttribs(SrcTex, RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
											   pTexArray, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
				CopyAttribs.SrcMipLevel = mip;
				CopyAttribs.DstMipLevel = mip;
				CopyAttribs.DstSlice    = i;
				AuroraEngine::ImmediateContext->CopyTexture(CopyAttribs);
			}
		}

		//AuroraEngine::ImmediateContext->GenerateMips(pTexArray->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

		return pTexArray;
	}

	RefCntAutoPtr<ITexture> GraphicUtilities::CreateRenderTarget2D(const char* name, int width, int height, const TEXTURE_FORMAT& format, const Vector4& clearColor, bool useAsShaderResource, bool useUav)
	{
		TextureDesc RTColorDesc;
		RTColorDesc.Name      = name;
		RTColorDesc.Type      = RESOURCE_DIM_TEX_2D;
		RTColorDesc.Width     = width;
		RTColorDesc.Height    = height;
		RTColorDesc.MipLevels = 1;
		RTColorDesc.Format    = format;
		// The render target can be bound as a shader resource and as a render target
		RTColorDesc.BindFlags = BIND_RENDER_TARGET;
		if(useAsShaderResource) {
			RTColorDesc.BindFlags |= BIND_SHADER_RESOURCE;
		}

		if(useUav) {
			RTColorDesc.BindFlags |= BIND_UNORDERED_ACCESS;
		}

		// Define optimal clear value
		RTColorDesc.ClearValue.Format   = RTColorDesc.Format;
		RTColorDesc.ClearValue.Color[0] = clearColor.x;
		RTColorDesc.ClearValue.Color[1] = clearColor.y;
		RTColorDesc.ClearValue.Color[2] = clearColor.z;
		RTColorDesc.ClearValue.Color[3] = clearColor.w;
		RefCntAutoPtr<ITexture> pRTColor;
		AuroraEngine::RenderDevice->CreateTexture(RTColorDesc, nullptr, &pRTColor);

		if(useUav) {

		}

		return pRTColor;
	}

	RefCntAutoPtr<ITexture> GraphicUtilities::CreateRenderTargetDepth2D(const char* name, int width, int height, const TEXTURE_FORMAT& format, bool useAsShaderResource, bool useUav)
	{
		TextureDesc desc;
		desc.Name      = name;
		desc.Type      = RESOURCE_DIM_TEX_2D;
		desc.Width     = width;
		desc.Height    = height;
		desc.MipLevels = 1;
		desc.Format    = format;
		desc.BindFlags = BIND_DEPTH_STENCIL;
		if(useAsShaderResource) {
			desc.BindFlags |= BIND_SHADER_RESOURCE;
		}

		if(useUav) {
			desc.BindFlags |= BIND_UNORDERED_ACCESS;
		}
		// Define optimal clear value
		desc.ClearValue.Format               = desc.Format;
		desc.ClearValue.DepthStencil.Depth   = 1;
		desc.ClearValue.DepthStencil.Stencil = 0;
		RefCntAutoPtr<ITexture> pRTDepth;
		AuroraEngine::RenderDevice->CreateTexture(desc, nullptr, &pRTDepth);

		return pRTDepth;
	}

	void GraphicUtilities::Blit(RefCntAutoPtr<ITexture> src, RefCntAutoPtr<ITexture> &dest)
	{
		Blit(m_BlitMaterial, src, dest);
	}

	void GraphicUtilities::Blit(std::shared_ptr<Material>& material, RefCntAutoPtr<ITexture> src, RefCntAutoPtr<ITexture> &dest)
	{
		ITextureView* view = dest->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
		AuroraEngine::ImmediateContext->SetRenderTargets(1, &view, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		GraphicsPipelineDesc& graphicsPipelineDesc = material->GetPipelineDesc();
		graphicsPipelineDesc.NumRenderTargets = 1;
		graphicsPipelineDesc.RTVFormats[0] = dest->GetDesc().Format;

		material->SetTexture("Texture", src);

		material->ValidateGraphicsPipelineState();
		material->ApplyPipeline();

		material->CommitShaderResources();

		DrawAttribs drawAttrs;
		drawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
		drawAttrs.NumVertices = 4;
		AuroraEngine::ImmediateContext->Draw(drawAttrs);
	}

	RefCntAutoPtr<ITexture> GraphicUtilities::GetPlaceholderTexture()
	{
		return m_PlaceholderTexture;
	}

	void GraphicUtilities::SetPlaceholderTexture(const RefCntAutoPtr<ITexture> &texture)
	{
		m_PlaceholderTexture = texture;
	}

	void GraphicUtilities::Blit(RefCntAutoPtr<ITexture> src, ITextureView *dest)
	{
		Blit(m_BlitMaterial, src, dest);
	}

	void GraphicUtilities::Blit(std::shared_ptr<Material> &material, RefCntAutoPtr<ITexture> src, ITextureView *dest)
	{
		AuroraEngine::ImmediateContext->SetRenderTargets(1, &dest, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		GraphicsPipelineDesc& graphicsPipelineDesc = material->GetPipelineDesc();
		graphicsPipelineDesc.NumRenderTargets = 1;
		graphicsPipelineDesc.RTVFormats[0] = dest->GetDesc().Format;

		material->SetTexture("Texture", src);

		material->ValidateGraphicsPipelineState();
		material->ApplyPipeline();

		material->CommitShaderResources();

		DrawAttribs drawAttrs;
		drawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
		drawAttrs.NumVertices = 4;
		AuroraEngine::ImmediateContext->Draw(drawAttrs);
	}

	void GraphicUtilities::Blit(std::shared_ptr<Material> &material, const std::map<String, RefCntAutoPtr<ITexture>> &srcTextures, ITextureView *dest)
	{
		AuroraEngine::ImmediateContext->SetRenderTargets(1, &dest, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		GraphicsPipelineDesc& graphicsPipelineDesc = material->GetPipelineDesc();
		graphicsPipelineDesc.NumRenderTargets = 1;
		graphicsPipelineDesc.RTVFormats[0] = dest->GetDesc().Format;

		for(auto& it : srcTextures) {
			material->SetTexture(it.first, it.second);
		}

		material->ValidateGraphicsPipelineState();
		material->ApplyPipeline();

		material->CommitShaderResources();

		DrawAttribs drawAttrs;
		drawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
		drawAttrs.NumVertices = 4;
		AuroraEngine::ImmediateContext->Draw(drawAttrs);
	}

	std::shared_ptr<Material> GraphicUtilities::Setup2DMaterial(std::shared_ptr<Material> material, bool useBlending)
	{
		material->SetCullMode(CULL_MODE_NONE);
		material->SetDepthEnable(false);
		material->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

		if(useBlending) {
			RenderTargetBlendDesc blendDesc;
			blendDesc.BlendEnable = true;
			blendDesc.SrcBlend = BLEND_FACTOR_SRC_ALPHA;
			blendDesc.DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;

			material->SetBlendState(blendDesc);
		}

		return material;
	}
}