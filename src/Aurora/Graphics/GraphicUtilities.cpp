#include "GraphicUtilities.hpp"
#include <Aurora/AuroraEngine.hpp>

using namespace Aurora;

namespace EmberSky
{
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
				std::cerr << "Cannot create texture array with different texture sizes ! (" << TexDesc.Name << ")" << std::endl;
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
}