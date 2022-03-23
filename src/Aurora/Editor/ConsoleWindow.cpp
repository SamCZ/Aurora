#include "ConsoleWindow.hpp"
#include "Aurora/Tools/ImGuiHelper.hpp"

namespace Aurora
{
	void ConsoleWindow::Draw()
	{
		if(!ImGui::Begin("Console"))
		{
			ImGui::End();
			return;
		}

		if(ImGui::IconButton("Clear"))
		{
			m_Messages.clear();
		}
		ImGui::SameLine();
		ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

		ImGui::Separator();

		// Reserve enough left-over height for 1 separator + 1 input text
		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

		for (const SMessage& message : m_Messages)
		{
			switch (message.Severity)
			{
				case Logger::Severity::Info:
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255));
					break;
				case Logger::Severity::Warning:
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(246,246, 33, 255));
					break;
				case Logger::Severity::Error:
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
					break;
				case Logger::Severity::FatalError:
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 20, 0, 255));
					break;
			}
			ImGui::Text("[%s] %s", message.SeverityStr.c_str(), message.Message.c_str());
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s: %s:%d", message.File.c_str(), message.Function.c_str(), message.Line);
			ImGui::PopStyleColor();
		}

		if (m_ScrollToBottom || (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
		{
			ImGui::SetScrollHereY(1.0f);
			m_ScrollToBottom = false;
		}

		ImGui::PopStyleVar();
		ImGui::EndChild();

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();

		ImGui::End();
	}

	void ConsoleWindow::Log(const Logger::Severity& severity, const std::string& severityStr, const std::string &file, const std::string &function, int line, const std::string &message)
	{
		m_Messages.push_back({severity, severityStr, file, function, line, message});
	}
}