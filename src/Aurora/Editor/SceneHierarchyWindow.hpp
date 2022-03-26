#pragma once

#include "Aurora/Tools/ImGuiHelper.hpp"
#include "Aurora/Tools/ImGuizmo.h"

namespace Aurora
{
	class MainEditorPanel;
	class Actor;

	class SceneHierarchyWindow
	{
	private:
		MainEditorPanel* m_MainEditorPanel;
	public:
		explicit SceneHierarchyWindow(MainEditorPanel* mainEditorPanel);

		void Update(double delta);

	private:
		void LookAtObject(Actor* actor);
	};
}