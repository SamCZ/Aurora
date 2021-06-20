#pragma once

#include <vector>
#include <array>
#include <memory>
#include <map>

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Common.hpp"

#include "Base/IRenderDevice.hpp"

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
		static Texture_ptr CreateTextureArray(const std::vector<Path>& textures);
		static Texture_ptr CreateCubeMap(const std::array<Texture_ptr, 6>& textures);
		static Texture_ptr CreateRenderTarget2D(const char* name, int width, int height, const GraphicsFormat& format, const Vector4& clearColor, bool useAsShaderResource, bool useUav = false);
		static Texture_ptr CreateRenderTargetDepth2D(const char* name, int width, int height, const GraphicsFormat& format, bool useAsShaderResource, bool useUav = false);

		static void Blit(Texture_ptr src, Texture_ptr dest);
		static void Blit(std::shared_ptr<Material>& material, Texture_ptr src, Texture_ptr dest);

		static void Blit(std::shared_ptr<Material>& material, const std::map<String, Texture_ptr>& srcTextures, Texture_ptr dest);

		static Texture_ptr GetPlaceholderTexture();
		static void SetPlaceholderTexture(Texture_ptr texture);

		static std::shared_ptr<Material> Setup2DMaterial(std::shared_ptr<Material> material, bool useBlending = false);
	};
}
