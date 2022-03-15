#pragma once

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Tools/ImGuiHelper.hpp"
#include "Aurora/Graphics/Base/Texture.hpp"

namespace Aurora::EUI
{
	static bool ImageButton(ITexture* texture, float size)
	{
		return ImGui::ImageButton((ImTextureID)texture->GetRawHandle(), { size , size }, { 0, 0 }, { 1, 1 });
	}

	static bool ImageButton(const Texture_ptr& texture, float size)
	{
		return ImageButton(texture.get(), size);
	}

	static void Image(ITexture* texture, const Vector2& size, bool flipY = false)
	{
		if(flipY || texture->GetDesc().IsRenderTarget)
			ImGui::Image((ImTextureID)texture->GetRawHandle(), { size.x , size.y }, { 0, 1 }, { 1, 0 });
		else
			ImGui::Image((ImTextureID)texture->GetRawHandle(), { size.x , size.y }, { 0, 0 }, { 1, 1 });
	}

	static void Image(const Texture_ptr& texture, const Vector2& size, bool flipY = false)
	{
		Image(texture.get(), size, flipY);
	}
}