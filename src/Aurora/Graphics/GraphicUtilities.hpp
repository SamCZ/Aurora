#pragma once

#include <vector>
#include <array>
#include <memory>
#include <map>

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Common.hpp"

#include "IRenderDevice.hpp"

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
		static TextureHandle CreateTextureArray(const std::vector<TextureHandle>& textures);
		static TextureHandle CreateCubeMap(const std::array<TextureHandle, 6>& textures);
		static TextureHandle CreateRenderTarget2D(const char* name, int width, int height, const Format::Enum& format, const Vector4& clearColor, bool useAsShaderResource, bool useUav = false);
		static TextureHandle CreateRenderTargetDepth2D(const char* name, int width, int height, const Format::Enum& format, bool useAsShaderResource, bool useUav = false);

		static void Blit(TextureHandle src, TextureHandle dest);
		static void Blit(std::shared_ptr<Material>& material, TextureHandle src, TextureHandle dest);

		static void Blit(std::shared_ptr<Material>& material, const std::map<String, TextureHandle>& srcTextures, TextureHandle dest);

		static TextureHandle GetPlaceholderTexture();
		static void SetPlaceholderTexture(TextureHandle texture);

		static std::shared_ptr<Material> Setup2DMaterial(std::shared_ptr<Material> material, bool useBlending = false);
	};
}
