#pragma once

#include <vector>
#include <array>
#include <Texture.h>
#include <RefCntAutoPtr.hpp>

using namespace Diligent;

namespace EmberSky
{
	class GraphicUtilities
	{
	public:
		static RefCntAutoPtr<ITexture> CreateTextureArray(const std::vector<RefCntAutoPtr<ITexture>>& textures);
		static RefCntAutoPtr<ITexture> CreateCubeMap(const std::array<RefCntAutoPtr<ITexture>, 6>& textures);
	};
}
