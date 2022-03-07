#pragma once

#include "Aurora/Core/Vector.hpp"

#define IM_VEC2_CLASS_EXTRA \
		ImVec2(const glm::vec2& f) { x = f.x; y = f.y; }\
		operator glm::vec2() const { return glm::vec2(x,y); }

#define IM_VEC4_CLASS_EXTRA \
		ImVec4(const glm::vec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }\
		operator glm::vec4() const { return glm::vec4(x,y,z,w); }

#include <imgui.h>
#include <imgui_internal.h>
#include <string>

namespace ImGui
{
	inline void SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}

	inline void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	inline bool Button(const std::string& label, const ImVec2& size_arg = ImVec2(0, 0))
	{
		return ButtonEx(label.c_str(), size_arg, ImGuiButtonFlags_None);
	}

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

	inline bool InputText(const std::string& label, std::string& text, bool enableLabel = true)
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
			return true;
		}
		return false;
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

	inline void BeginChild(const char* name, float width, float height, bool border, float padding = -1, ImGuiWindowFlags flags = 0) {
		if (padding >= 0) {
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { padding, padding });
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { padding, padding });
		}
		ImGui::BeginChild(name, { width, height }, border, flags);
		if (padding >= 0) {
			ImGui::PopStyleVar();
		}
	}

	inline void BeginWindow(const std::string& name, float x, float y, float width, float height, bool stay = false, int padding = -1, bool* p_open = nullptr) {
		if (padding >= 0) {
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { (float)padding, (float)padding });
		}
		ImGui::SetNextWindowPos(ImVec2(x, y), stay ? 0 : ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(width, height), stay ? 0 : ImGuiCond_FirstUseEver);
		ImGui::Begin(name.c_str(), p_open, stay ? (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize) : 0);
		if (padding >= 0) {
			ImGui::PopStyleVar();
		}
	}

	inline void EndWindow() {
		ImGui::End();
	}
}

namespace ImGui
{
	void RenderHollowBullet(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float thickness = 1.0f);

	void Bullet(bool hollow);
	void Bullet(bool hollow, glm::vec4 color);
}

namespace ImGui
{
	void NodePin(bool fill, glm::vec4 color);
	void NodePin_Execute(bool fill, glm::vec4 color);
}
