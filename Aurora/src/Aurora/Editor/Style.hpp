#pragma once

#include "Aurora/Tools/ImGuiHelper.hpp"

namespace Aurora
{
	// Style Colors
	glm::vec4 s_MainBgLight0{ 0.404f, 0.404f ,0.404f, 1.0f };
	glm::vec4 s_MainBg{ 0.21f, 0.21f, 0.21f, 1.0f };
	glm::vec4 s_MainBgDark0{ 0.190f, 0.190f, 0.190f, 1.0f };
	glm::vec4 s_MainBgDark1{ 0.145f, 0.145f, 0.145f, 1.0f };
	glm::vec4 s_MainBgDark2{ 0.098f, 0.098f, 0.098f, 1.0f };

	glm::vec4 s_Accent{ 0.149f, 0.149f, 0.149f, 1.0f };
	glm::vec4 s_AccentDark0{ 0.102f, 0.102f, 0.102f, 1.0f };
	glm::vec4 s_AccentDark1{ 0.063f, 0.063f, 0.063f, 1.0f };

	glm::vec4 s_Button{ 0.882f, 0.882f, 0.882f, 1.0f };
	glm::vec4 s_ButtonHovered{ 0.782f, 0.782f, 0.782f, 1.0f };

	glm::vec4 s_Header{ 0.338f, 0.338f, 0.338f, 1.0f };
	glm::vec4 s_HeaderHovered{ 0.276f, 0.276f, 0.276f, 1.0f };
	glm::vec4 s_HeaderActive{ 0.379f, 0.379f, 0.379f, 1.0f };

	glm::vec4 s_Font{ 0.902f, 0.902f, 0.902f, 1.0f };
	glm::vec4 s_FontDisabled{ 0.36f, 0.36f, 0.36f, 1.0f };
	glm::vec4 s_HighlightColor{ 0.145f, 0.553f, 0.384f, 1.0f };

	// Sizing
	glm::vec2 s_WindowPadding{ 10, 10 };
	glm::vec2 s_FramePadding{ 20, 8 };
	glm::vec2 s_ItemSpacing{ 20, 8 };
	float s_ScrollbarSize = 17;
	float s_ScrollbarRounding = 12;
	float s_FrameRounding = 8;
	float s_GrabRounding = 8;
	float s_TabRounding = 8;

	void SetEditorStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		// Apply Colors
		style.Colors[ImGuiCol_WindowBg] = s_MainBg;
		style.Colors[ImGuiCol_ChildBg] = s_MainBg;

		style.Colors[ImGuiCol_Text] = s_Font;
		style.Colors[ImGuiCol_TextDisabled] = s_FontDisabled;
		style.Colors[ImGuiCol_TextSelectedBg] = s_MainBgDark1;

		style.Colors[ImGuiCol_FrameBg] = s_MainBgDark1;
		style.Colors[ImGuiCol_FrameBgHovered] = s_MainBgDark0;
		style.Colors[ImGuiCol_FrameBgActive] = s_MainBgDark2;

		style.Colors[ImGuiCol_TitleBg] = s_MainBgDark0;
		style.Colors[ImGuiCol_TitleBgCollapsed] = s_MainBgDark0;
		style.Colors[ImGuiCol_TitleBgActive] = s_MainBgDark0;
		style.Colors[ImGuiCol_MenuBarBg] = s_MainBgDark0;
		style.Colors[ImGuiCol_MenuBarBg] = s_AccentDark0;
		//style.Colors[ImGuiCol_MenuBarButtonBgHover] = s_AccentDark1;
		//style.Colors[ImGuiCol_MenuBarButtonBgActive] = s_AccentDark1;

		style.Colors[ImGuiCol_Tab] = s_MainBgDark0;
		style.Colors[ImGuiCol_TabUnfocused] = s_MainBgDark0;
		style.Colors[ImGuiCol_TabHovered] = s_MainBgDark1;
		style.Colors[ImGuiCol_TabActive] = s_MainBgDark1;
		style.Colors[ImGuiCol_TabUnfocusedActive] = s_MainBgDark1;

		style.Colors[ImGuiCol_ScrollbarBg] = s_MainBgDark1;
		style.Colors[ImGuiCol_ScrollbarGrab] = s_Font;
		style.Colors[ImGuiCol_ScrollbarGrabActive] = s_FontDisabled;
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = s_FontDisabled;
		style.Colors[ImGuiCol_CheckMark] = s_Font;
		style.Colors[ImGuiCol_SliderGrab] = s_Font;
		style.Colors[ImGuiCol_SliderGrabActive] = s_FontDisabled;

		/*style.Colors[ImGuiCol_Button] = s_Button;
		style.Colors[ImGuiCol_ButtonHovered] = s_ButtonHovered;
		style.Colors[ImGuiCol_ButtonActive] = s_ButtonHovered;*/

		style.Colors[ImGuiCol_Header] = s_Header;
		style.Colors[ImGuiCol_HeaderHovered] = s_HeaderHovered;
		style.Colors[ImGuiCol_HeaderActive] = s_HeaderActive;

		style.Colors[ImGuiCol_Separator] = s_MainBgLight0;
		style.Colors[ImGuiCol_SeparatorHovered] = s_MainBgLight0;
		style.Colors[ImGuiCol_SeparatorActive] = s_MainBgLight0;
		style.Colors[ImGuiCol_Border] = s_MainBgLight0;

		style.Colors[ImGuiCol_ResizeGrip] = s_MainBg;
		style.Colors[ImGuiCol_ResizeGripHovered] = s_MainBg;
		style.Colors[ImGuiCol_ResizeGripActive] = s_MainBg;

		style.Colors[ImGuiCol_DockingPreview] = s_AccentDark0;
		style.Colors[ImGuiCol_TextSelectedBg] = s_HighlightColor;
		style.Colors[ImGuiCol_NavHighlight] = s_AccentDark0;
	}
}