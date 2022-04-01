#pragma once

#include "WindowBase.hpp"

namespace Aurora
{
	class MainEditorPanel;
	class MeshComponent;

	class PropertiesWindow : public EditorWindowBase
	{
	private:
		MainEditorPanel* m_MainPanel;
	public:
		explicit PropertiesWindow(MainEditorPanel* mainEditorPanel) : EditorWindowBase("Properties", true, true), m_MainPanel(mainEditorPanel) {}

		void OnGui() override;

	private:
		void DrawComponentGui(MeshComponent* component);
	};
}
