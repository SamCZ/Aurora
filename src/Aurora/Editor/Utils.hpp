#pragma once

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Tools/ImGuiHelper.hpp"
#include "Aurora/Tools/IconsFontAwesome5.hpp"
#include "Aurora/Graphics/Base/Texture.hpp"

#include <imgui_internal.h>

#ifndef ARSIZE
#define ARSIZE(arr) (sizeof(arr)/sizeof((arr)[0]))
#define ENUM_COMBO(name, current, enum_name) ImGui::Combo(name, current, enum_name##_Strings, ARSIZE(enum_name##_Strings))
#endif
namespace Aurora
{
	class ActorComponent;
}

namespace Aurora::EUI
{
	static bool ImageButton(ITexture* texture, float size)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		bool pressed = ImGui::ImageButton((ImTextureID)texture->GetRawHandle(), { size , size }, { 0, 0 }, { 1, 1 }, 0, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
		ImGui::PopStyleColor();
		return pressed;
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

	const char* GetIconForComponent(ActorComponent* component);

	static bool Slot(const String& name, const char* icon, bool* clearClicked = nullptr)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

		ImGui::BeginGroup();
		bool clicked = ImGui::Button(icon, ImVec2(63, 63));
		ImGui::SameLine();
		ImGui::Text("%s", name.c_str());
		ImGui::EndGroup();
		ImGui::PopStyleColor();

		if (!clearClicked)
			return clicked;

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("SlotPopup");
		}

		if (ImGui::BeginPopup("SlotPopup"))
		{
			*clearClicked = ImGui::Button("Clear");
			ImGui::EndPopup();
		}

		return clicked;
	}

	static bool Slot(const String& name, ITexture* texture, bool* clearClicked = nullptr)
	{
		if (!texture)
			return Slot(name, "None", clearClicked);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

		ImGui::BeginGroup();
		bool clicked = ImageButton(texture, 55);

		ImGui::SameLine();
		ImGui::Text("%s", name.c_str());

		//ImGui::SameLine(-10);
		//ImGui::Button("x");
		ImGui::EndGroup();
		ImGui::PopStyleColor();

		if (!clearClicked)
			return clicked;

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("SlotPopup");
		}

		if (ImGui::BeginPopup("SlotPopup"))
		{
			*clearClicked = ImGui::Button("Clear");
			ImGui::EndPopup();
		}

		return clicked;
	}

	Texture_ptr AcceptTextureFileDrop();
	Texture_ptr AcceptCubeMapTextureFileDrop();
}