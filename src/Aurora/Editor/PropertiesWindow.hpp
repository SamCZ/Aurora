#pragma once

#include "WindowBase.hpp"
#include "Aurora/Framework/Transform.hpp"

namespace Aurora
{
	class MainEditorPanel;
	class MeshComponent;
	class LightComponent;
	class PointLightComponent;

	class PropertiesWindow : public EditorWindowBase
	{
	private:
		MainEditorPanel* m_MainPanel;
		Transform m_TransformToCopy;
		bool m_IsTransformBeingCopied;
	public:
		explicit PropertiesWindow(MainEditorPanel* mainEditorPanel)
		: EditorWindowBase("Properties", true, true), m_MainPanel(mainEditorPanel), m_IsTransformBeingCopied(false) {}

		void OnGui() override;

	private:
		void DrawComponentGui(MeshComponent* component);
		void DrawComponentGui(LightComponent* component);
		void DrawComponentGui(PointLightComponent* component);
	};
}
