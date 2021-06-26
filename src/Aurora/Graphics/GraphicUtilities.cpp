#include "GraphicUtilities.hpp"
#include <Aurora/AuroraEngine.hpp>

#include "Material.hpp"

#include "Aurora/Assets/Tools/stb_image.h"
#include "Aurora/Assets/Tools/stb_image_resize.h"

namespace Aurora
{
	static Material_ptr m_BlitMaterial = nullptr;
	static Texture_ptr m_PlaceholderTexture;

	void GraphicUtilities::Init()
	{
		/*{
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
		}*/

		m_BlitMaterial = Setup2DMaterial(std::make_shared<Material>("Blit", "Assets/Shaders/Blit"), true);
	}

	void GraphicUtilities::Destroy()
	{
		//m_BlitMaterial.reset();
		//m_PlaceholderTexture.Release();
	}

	unsigned int GetMipLevelsNum(unsigned int width, unsigned int height)
	{
		unsigned int size = std::max<unsigned int>(width, height);
		unsigned int levelsNum = (unsigned int)(logf((float)size) / logf(2.0f)) + 1;

		return levelsNum;
	}

	Texture_ptr GraphicUtilities::CreateTextureArray(const std::vector<Path>& textures)
	{
		if(textures.empty()) {
			AU_LOG_WARNING("Provided empty list of textures");
			return nullptr;
		}

		Texture_ptr pTexArray = nullptr;

		int targetWidth = 0;
		int targetHeight = 0;
		bool targetSizeInitialized = false;

		for (int i = 0; i < textures.size(); ++i) {
			const Path& path = textures[i];

			auto fileData = ASM->LoadFile(path);

			if(fileData.empty()) {
				AU_LOG_FATAL("Could not load ", path);
			}

			int width,height,channels_in_file;
			unsigned char *data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(fileData.data()), fileData.size(), &width, &height, &channels_in_file, STBI_rgb_alpha);
			if(!data) {
				AU_LOG_ERROR("Cannot load texture !", path.string());
				return nullptr;
			}

			if(!targetSizeInitialized) {
				targetSizeInitialized = true;
				targetWidth = width;
				targetHeight = height;

				TextureDesc textureDesc;
				textureDesc.DebugName = "TextureArray";
				textureDesc.Width = targetWidth;
				textureDesc.Height = targetHeight;
				textureDesc.MipLevels = GetMipLevelsNum(targetWidth, targetHeight);
				textureDesc.IsArray = true;
				textureDesc.DepthOrArraySize = textures.size();
				textureDesc.ImageFormat = GraphicsFormat::RGBA8_UNORM;

				pTexArray = RD->CreateTexture(textureDesc);
			}

			if(width != targetWidth || height != targetHeight) {
				auto* resizedData = new uint8_t[targetWidth * targetHeight * STBI_rgb_alpha];
				stbir_resize_uint8(data, width, height, 0, resizedData, targetWidth, targetHeight, 0, STBI_rgb_alpha);
				stbi_image_free(data);
				data = resizedData;
				width = targetWidth;
				height = targetHeight;
			}

			for (unsigned int mipLevel = 0; mipLevel < pTexArray->GetDesc().MipLevels; mipLevel++)
			{
				AuroraEngine::RenderDevice->WriteTexture(pTexArray, mipLevel, i, data);

				if (mipLevel < pTexArray->GetDesc().MipLevels - 1u)
				{
					int newWidth = std::max<int>(1, width >> 1);
					int newHeight = std::max<int>(1, height >> 1);

					auto* resizedData = new uint8_t[newWidth * newHeight * STBI_rgb_alpha];
					stbir_resize_uint8(data, width, height, 0, resizedData, newWidth, newHeight, 0, STBI_rgb_alpha);

					stbi_image_free(data);
					data = resizedData;
					width = newWidth;
					height = newHeight;
				}
			}

			stbi_image_free(data);

			/*RD->WriteTexture(pTexArray, 0, i, data);
			stbi_image_free(data);*/
		}

		//RD->GenerateMipmaps(pTexArray);


		/*for (int i = 0; i < textures.size(); ++i) {
			TextureHandle SrcTex = textures[i];
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
				AU_LOG_ERROR("Cannot create texture array with different texture sizes ! (", TexDesc.Name, ")")
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
		}*/

		//AuroraEngine::ImmediateContext->GenerateMips(pTexArray->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

		return pTexArray;
	}

	Texture_ptr GraphicUtilities::CreateCubeMap(const std::array<Path, 6>& textures)
	{
		Texture_ptr pTexArray = nullptr;

		int targetWidth = 0;
		int targetHeight = 0;
		bool targetSizeInitialized = false;

		for (int i = 0; i < textures.size(); ++i) {
			const Path& path = textures[i];

			auto fileData = ASM->LoadFile(path);

			if(fileData.empty()) {
				AU_LOG_FATAL("Could not load ", path);
			}

			int width,height,channels_in_file;
			unsigned char *data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(fileData.data()), fileData.size(), &width, &height, &channels_in_file, STBI_rgb_alpha);
			if(!data) {
				AU_LOG_ERROR("Cannot load texture !", path.string());
				return nullptr;
			}

			if(!targetSizeInitialized) {
				targetSizeInitialized = true;
				targetWidth = width;
				targetHeight = height;

				TextureDesc textureDesc;
				textureDesc.DebugName = "CubeMap";
				textureDesc.Width = targetWidth;
				textureDesc.Height = targetHeight;
				textureDesc.MipLevels = 1;
				textureDesc.IsCubeMap = true;
				textureDesc.DepthOrArraySize = 6;
				textureDesc.ImageFormat = GraphicsFormat::RGBA8_UNORM;

				pTexArray = RD->CreateTexture(textureDesc);
			}

			if(width != targetWidth || height != targetHeight) {
				auto* resizedData = new uint8_t[targetWidth * targetHeight * STBI_rgb_alpha];
				stbir_resize_uint8(data, width, height, 0, resizedData, targetWidth, targetHeight, 0, STBI_rgb_alpha);
				stbi_image_free(data);
				data = resizedData;
				width = targetWidth;
				height = targetHeight;
			}

			for (unsigned int mipLevel = 0; mipLevel < pTexArray->GetDesc().MipLevels; mipLevel++)
			{
				AuroraEngine::RenderDevice->WriteTexture(pTexArray, mipLevel, i, data);

				if (mipLevel < pTexArray->GetDesc().MipLevels - 1u)
				{
					int newWidth = std::max<int>(1, width >> 1);
					int newHeight = std::max<int>(1, height >> 1);

					auto* resizedData = new uint8_t[newWidth * newHeight * STBI_rgb_alpha];
					stbir_resize_uint8(data, width, height, 0, resizedData, newWidth, newHeight, 0, STBI_rgb_alpha);

					stbi_image_free(data);
					data = resizedData;
					width = newWidth;
					height = newHeight;
				}
			}

			stbi_image_free(data);

			/*RD->WriteTexture(pTexArray, 0, i, data);
			stbi_image_free(data);*/
		}

		/*for (int i = 0; i < 6; ++i) {
			TextureHandle SrcTex = textures[i];
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
		}*/

		//AuroraEngine::ImmediateContext->GenerateMips(pTexArray->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

		return pTexArray;
	}

	Texture_ptr GraphicUtilities::CreateRenderTarget2D(const char* name, int width, int height, const GraphicsFormat& format, const Vector4& clearColor, bool useAsShaderResource, bool useUav)
	{
		TextureDesc gBufferDesc;
		gBufferDesc.Width = width;
		gBufferDesc.Height = height;
		gBufferDesc.IsRenderTarget = true;
		gBufferDesc.UseClearValue = true;
		gBufferDesc.SampleCount = 1;
		gBufferDesc.DisableGPUsSync = true;
		gBufferDesc.IsUAV = useUav;

		gBufferDesc.ImageFormat = format;
		gBufferDesc.ClearValue = clearColor;
		gBufferDesc.DebugName = name;

		return AuroraEngine::RenderDevice->CreateTexture(gBufferDesc, nullptr);
	}

	Texture_ptr GraphicUtilities::CreateRenderTargetDepth2D(const char* name, int width, int height, const GraphicsFormat& format, bool useAsShaderResource, bool useUav)
	{
		return CreateRenderTarget2D(name, width, height, format, Vector4(1, 1, 1, 1), useAsShaderResource, useUav);
	}

	void GraphicUtilities::Blit(Texture_ptr src, Texture_ptr dest)
	{
		Blit(m_BlitMaterial, src, dest);
	}

	void GraphicUtilities::Blit(std::shared_ptr<Material>& material, Texture_ptr src, Texture_ptr dest)
	{
		/*ITextureView* view = dest->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
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
		AuroraEngine::ImmediateContext->Draw(drawAttrs);*/
	}

	Texture_ptr GraphicUtilities::GetPlaceholderTexture()
	{
		return m_PlaceholderTexture;
	}

	void GraphicUtilities::SetPlaceholderTexture(Texture_ptr texture)
	{
		m_PlaceholderTexture = texture;
	}

	void GraphicUtilities::Blit(std::shared_ptr<Material> &material, const std::map<String, Texture_ptr> &srcTextures, const Texture_ptr& dest)
	{
		DrawCallState drawCallState;

		if(dest != nullptr) {
			drawCallState.BindTarget(0, dest);
		}

		material->Apply(drawCallState);

		for (const auto &item : srcTextures) {
			drawCallState.BindTexture(item.first, item.second);
		}

		RD->Draw(drawCallState, {DrawArguments(4)});
	}

	std::shared_ptr<Material> GraphicUtilities::Setup2DMaterial(std::shared_ptr<Material> material, bool useBlending)
	{
		material->SetCullMode(ECullMode::None);
		material->SetDepthEnable(false);
		material->SetPrimitiveTopology(EPrimitiveType::TriangleStrip);

		/*material->SetCullMode(RasterState::CullMode::None);
		material->SetDepthEnable(false);
		material->SetPrimitiveTopology(PrimitiveType::TriangleStrip);

		if(useBlending) {
			BlendState blendState;

			for (uint32_t i = 0; i < BlendState::MAX_MRT_BLEND_COUNT; i++)
			{
				blendState.blendEnable[i] = true;
				blendState.colorWriteEnable[i] = BlendState::COLOR_MASK_ALL;

				// TODO: Check if this right settings for alpha
				blendState.srcBlend[i] = BlendState::BLEND_SRC_ALPHA;
				blendState.destBlend[i] = BlendState::BLEND_INV_SRC_ALPHA;
				blendState.blendOp[i] = BlendState::BLEND_OP_ADD;

				blendState.srcBlendAlpha[i] = BlendState::BLEND_ONE;
				blendState.destBlendAlpha[i] = BlendState::BLEND_ZERO;
				blendState.blendOpAlpha[i] = BlendState::BLEND_OP_ADD;
			}

			material->SetBlendState(blendState);
		}*/

		return material;
	}
}