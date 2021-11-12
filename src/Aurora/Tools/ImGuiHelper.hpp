#pragma once

//#include <ImGuiUtils.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <string>
#include "Aurora/Core/Vector.hpp"

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

	void DrawSplitter(int split_vertically, float thickness, float* size0, float* size1, float min_size0, float min_size1)
	{
		ImVec2 backup_pos = ImGui::GetCursorPos();
		if (split_vertically)
			ImGui::SetCursorPosY(backup_pos.y + *size0);
		else
			ImGui::SetCursorPosX(backup_pos.x + *size0);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,0));          // We don't draw while active/pressed because as we move the panes the splitter button will be 1 frame late
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f,0.6f,0.6f,0.10f));
		ImGui::Button("##Splitter", ImVec2(!split_vertically ? thickness : -1.0f, split_vertically ? thickness : -1.0f));
		ImGui::PopStyleColor(3);

		ImGui::SetItemAllowOverlap(); // This is to allow having other buttons OVER our splitter.

		if (ImGui::IsItemActive())
		{
			float mouse_delta = split_vertically ? ImGui::GetIO().MouseDelta.y : ImGui::GetIO().MouseDelta.x;

			// Minimum pane size
			if (mouse_delta < min_size0 - *size0)
				mouse_delta = min_size0 - *size0;
			if (mouse_delta > *size1 - min_size1)
				mouse_delta = *size1 - min_size1;

			// Apply resize
			*size0 += mouse_delta;
			*size1 -= mouse_delta;
		}
		ImGui::SetCursorPos(backup_pos);
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

	inline bool IsItemActivePreviousFrame()
	{
		ImGuiContext& g = *GImGui;
		if (g.ActiveIdPreviousFrame)
			return g.ActiveIdPreviousFrame== GImGui->CurrentWindow->DC.LastItemId;
		return false;
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