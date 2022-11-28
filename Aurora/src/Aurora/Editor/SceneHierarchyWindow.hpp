#pragma once

#include "Aurora/Tools/ImGuiHelper.hpp"
#include "Aurora/Tools/ImGuizmo.h"

namespace Aurora
{
	class MainEditorPanel;
	class Actor;
	class SceneComponent;
	class ActorComponent;

	class SceneHierarchyWindow
	{
	private:
		MainEditorPanel* m_MainEditorPanel;
	public:
		explicit SceneHierarchyWindow(MainEditorPanel* mainEditorPanel);

		void Update(double delta);

	private:
		void DrawSceneComponent(int& ID, SceneComponent* actorComponent, bool opened);
		void DrawActorComponent(int& ID, ActorComponent* actorComponent, bool opened);

		void LookAtObject(Actor* actor);
	};
}