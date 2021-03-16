#pragma once

#include <ImGuiUtils.hpp>
#include <imgui_internal.h>
#include <string>
#include "Aurora/Core/Vector.hpp"

namespace ImGui
{
	inline bool Selectable(const std::string& label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0,0))
	{
		return ImGui::Selectable(label.c_str(), selected, flags, size);
	}

	inline bool BeginWindowNoBg(const Vector2& pos, const Vector2& size, bool enableBg = false, float opacity = 1.0)
	{
		ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_Always);

		int flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
		if(!enableBg) flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
		bool state = ImGui::Begin("Debug info", nullptr, flags); //TODO: Change id
		ImGui::PopStyleVar();
		return state;
	}

	inline void InputText(const std::string& label, std::string& text, bool enableLabel = true)
	{
		if(enableLabel) {
			ImGui::Text("%s", label.c_str());
			ImGui::SameLine();
		}

		static char name[64];
		memset(name, 0x00, 64);
		text.copy(name, 64);
		if (ImGui::InputText(("##" + label).c_str(), name, 64)) {
			text = std::string(name);
		}
	}

	inline void InputInt(const std::string& label, int& i)
	{
		ImGui::Text("%s", label.c_str());
		ImGui::SameLine();

		ImGui::InputInt(("##" + label).c_str(), &i, 1);
	}

	inline void Text(const std::string& text) {
		ImGui::Text("%s", text.c_str());
	}

	inline void BeginChild(const char* name, float width, float height, bool border, float padding = -1) {
		if (padding >= 0) {
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { padding, padding });
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { padding, padding });
		}
		ImGui::BeginChild(name, { width, height }, border);
		if (padding >= 0) {
			ImGui::PopStyleVar();
		}
	}

	inline bool IsItemActivePreviousFrame()
	{
		ImGuiContext& g = *GImGui;
		if (g.ActiveIdPreviousFrame)
			return g.ActiveIdPreviousFrame== GImGui->CurrentWindow->DC.LastItemId;
		return false;
	}
}