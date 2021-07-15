#include "Profiler.hpp"

#ifdef AU_PROFILER_ENABLED
#include <iostream>
#endif

#include <imgui.h>
#include <Aurora/App/ImGuiHelper.hpp>

namespace Aurora
{
	Profiler::ProfileNode Profiler::rootNode("Root"); // NOLINT(cert-err58-cpp)
	Profiler::ProfileNode Profiler::m_LastFrameRootNode("Root"); // NOLINT(cert-err58-cpp)
	Profiler::ProfileNode* Profiler::m_CurrentNode(nullptr);
	bool Profiler::m_IsPaused(false);

#ifdef AU_PROFILER_ENABLED

	void Profiler::Begin(const std::string &name)
	{
		if(name == "Root") {
			return;
		}

		ProfileNode node(name);
		node.ParentNode = m_CurrentNode;
		node.Begin();

		m_CurrentNode = &m_CurrentNode->Childs.emplace_back(node);
	}

	void Profiler::End(const std::string &name)
	{

		if(name == "Root") {
			return;
		}

		if(m_CurrentNode->Name != name) {
			std::cerr << "[Profiler] Name " << name << " does not exists !" << std::endl;
			m_CurrentNode = &rootNode;
			return;
		}

		m_CurrentNode->End();
		m_CurrentNode = m_CurrentNode->ParentNode;
	}

	void Profiler::RestartProfiler()
	{
		rootNode.Clear();
		rootNode.Begin();

		m_CurrentNode = &rootNode;
	}

	void Profiler::Finalize()
	{
		rootNode.End();

		if(rootNode.Name != m_CurrentNode->Name) {
			std::cerr << "[Profiler] Finalize error! RootNode(" << rootNode.Name << ") and CurrentNode(" << m_CurrentNode->Name << ") does not match !";
		}

		if(!m_IsPaused) {
			m_LastFrameRootNode = rootNode;
			m_LastFrameRootNode.UpdateRootNodes();
		}
	}

	void DrawNode(const Profiler::ProfileNode& node)
	{
		std::string name = node.Name + " - " + std::to_string(node.GetElapsedTimeMillis()) + "ms###" + node.Name;

		static bool test = false;

		if(ImGui::TreeNodeEx(name.c_str(), node.Childs.size() > 15 ? 0 : ImGuiTreeNodeFlags_DefaultOpen)) {
			for(const Profiler::ProfileNode& child : node.Childs) {
				DrawNode(child);
			}

			ImGui::TreePop();
		}
	}

	void Profiler::DrawWithImGui(bool createWindow)
	{
		if(createWindow) {
			static bool profileWindowOpened = false;
			ImGui::BeginWindow("Profiler window", ImGui::GetIO().DisplaySize.x - 550, 0, 550, 600, false, 5, &profileWindowOpened);
		}

		ImGui::Checkbox("Paused###ProfilerPaused", &m_IsPaused);

		DrawNode(m_LastFrameRootNode);

		if(true) { // Draw frame time graph
			static float values[120] = {};
			static int values_offset = 0;

			values[values_offset] = m_LastFrameRootNode.GetElapsedTimeMillis();
			values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);

			float average = 0.0f;
			for (int n = 0; n < IM_ARRAYSIZE(values); n++)
				average += values[n];
			average /= (float)IM_ARRAYSIZE(values);
			char overlay[32];
			sprintf(overlay, "avg %f", average);
			ImGui::PlotHistogram("###FrameTimeGraph", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0f, average + 10.0f, ImVec2(0, 80.0f));
		}

		if(createWindow) {
			ImGui::EndWindow();
		}
	}

#endif
}