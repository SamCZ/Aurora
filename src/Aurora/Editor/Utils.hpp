#pragma once

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Tools/ImGuiHelper.hpp"
#include "Aurora/Graphics/Base/Texture.hpp"

#include <imgui_internal.h>

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

	static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
	{
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter2");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
		bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
		return ImGui::SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}
}