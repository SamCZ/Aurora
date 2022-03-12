#include "ImGuiHelper.hpp"

namespace ImGui
{
	void RenderHollowBullet(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float thickness)
	{
		draw_list->AddCircle(pos, draw_list->_Data->FontSize * 0.20f, col, 8, thickness);
	}

	void Bullet(bool hollow)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const float line_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + g.Style.FramePadding.y * 2), g.FontSize);
		const ImRect bb(window->DC.CursorPos, glm::vec2(window->DC.CursorPos) + glm::vec2(g.FontSize, line_height));
		ItemSize(bb);
		if (!ItemAdd(bb, 0))
		{
			SameLine(0, style.FramePadding.x * 2);
			return;
		}

		// Render and stay on same line
		ImU32 text_col = GetColorU32(ImGuiCol_Text);
		if(hollow)
			RenderHollowBullet(window->DrawList, glm::vec2(bb.Min) + glm::vec2(style.FramePadding.x + g.FontSize * 0.5f, line_height * 0.5f), text_col);
		else
			RenderBullet(      window->DrawList, glm::vec2(bb.Min) + glm::vec2(style.FramePadding.x + g.FontSize * 0.5f, line_height * 0.5f), text_col);
		SameLine(0, style.FramePadding.x * 2.0f);
	}

	void Bullet(bool hollow, glm::vec4 color)
	{
		PushStyleColor(ImGuiCol_Text, color);
		{
			Bullet(hollow);
		}
		PopStyleColor();
	}

	bool IconCheckbox(const char* name, bool* v)
	{
		PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

		ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		if(v && !*v)
		{
			color = ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
		}

		PushStyleColor(ImGuiCol_Text, color);

		bool clicked = Button(name);
		PopStyleColor(4);
		PopStyleVar();

		if(clicked && v)
		{
			*v = !*v;
		}

		return clicked;
	}
}

namespace ImGui
{
	void NodePin(bool fill, glm::vec4 color)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const float line_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + g.Style.FramePadding.y * 2), g.FontSize);
		const ImRect bb(window->DC.CursorPos, glm::vec2(window->DC.CursorPos) + glm::vec2(g.FontSize, line_height));
		ItemSize(bb);
		if (!ItemAdd(bb, 0))
		{
			SameLine(0, style.FramePadding.x * 2);
			return;
		}

		// Render and stay on same line
		ImU32 text_col = ColorConvertFloat4ToU32(color);
		if(fill)
		{
			window->DrawList->AddCircleFilled(
				glm::vec2(bb.Min) + glm::vec2(g.FontSize * 0.5f, line_height * 0.5f),
				window->DrawList->_Data->FontSize * 0.20f,
				text_col,
				8
			);
		}
		else
		{
			window->DrawList->AddCircle(
				glm::vec2(bb.Min) + glm::vec2(g.FontSize * 0.5f, line_height * 0.5f),
				window->DrawList->_Data->FontSize * 0.20f,
				text_col,
				8,
				1.0f
			);
		}
	}

	void NodePin_Execute(bool fill, glm::vec4 color)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const float line_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + g.Style.FramePadding.y * 2), g.FontSize);
		const ImRect bb(window->DC.CursorPos, glm::vec2(window->DC.CursorPos) + glm::vec2(g.FontSize, line_height));
		ItemSize(bb);
		if (!ItemAdd(bb, 0))
		{
			SameLine(0, style.FramePadding.x * 2);
			return;
		}

		// Render and stay on same line
		ImU32 text_col = ColorConvertFloat4ToU32(color);
		glm::vec2 p0 = glm::vec2(bb.Min) + glm::vec2(g.FontSize * 0.1f, line_height * 0.9f); // Up
		glm::vec2 p1 = glm::vec2(bb.Min) + glm::vec2(g.FontSize * 0.8f, line_height * 0.5f); // Right
		glm::vec2 p2 = glm::vec2(bb.Min) + glm::vec2(g.FontSize * 0.1f, line_height * 0.1f); // Up
		if(fill)
			window->DrawList->AddTriangleFilled(p0, p1, p2, text_col);
		else
			window->DrawList->AddTriangle(p0, p1, p2, text_col);
	}
}