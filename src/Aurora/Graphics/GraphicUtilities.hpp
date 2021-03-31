#pragma once

#include <vector>
#include <array>
#include <Texture.h>
#include <RefCntAutoPtr.hpp>
#include <memory>
#include <map>

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Common.hpp"

using namespace Diligent;

namespace Aurora
{
	class Material;

	class GraphicUtilities
	{
		friend class AuroraEngine;
	public:
		static void Init();
		static void Destroy();
	public:
		static RefCntAutoPtr<ITexture> CreateTextureArray(const std::vector<RefCntAutoPtr<ITexture>>& textures);
		static RefCntAutoPtr<ITexture> CreateCubeMap(const std::array<RefCntAutoPtr<ITexture>, 6>& textures);
		static RefCntAutoPtr<ITexture> CreateRenderTarget2D(const char* name, int width, int height, const TEXTURE_FORMAT& format, const Vector4& clearColor, bool useAsShaderResource, bool useUav = false);
		static RefCntAutoPtr<ITexture> CreateRenderTargetDepth2D(const char* name, int width, int height, const TEXTURE_FORMAT& format, bool useAsShaderResource, bool useUav = false);

		static void Blit(RefCntAutoPtr<ITexture>& src, RefCntAutoPtr<ITexture>& dest);
		static void Blit(RefCntAutoPtr<ITexture> & src, ITextureView* dest);
		static void Blit(std::shared_ptr<Material>& material, RefCntAutoPtr<ITexture>& src, RefCntAutoPtr<ITexture>& dest);
		static void Blit(std::shared_ptr<Material>& material, RefCntAutoPtr<ITexture> & src, ITextureView* dest);

		static void Blit(std::shared_ptr<Material>& material, const std::map<String, RefCntAutoPtr<ITexture>>& srcTextures, ITextureView* dest);

		static RefCntAutoPtr<ITexture> GetPlaceholderTexture();
		static void SetPlaceholderTexture(const RefCntAutoPtr<ITexture>& texture);

		static std::shared_ptr<Material> Setup2DMaterial(std::shared_ptr<Material> material, bool useBlending = false);
	};
}
