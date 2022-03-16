#pragma once

#include "ConsoleWindow.hpp"

#include "Aurora/Graphics/Base/Texture.hpp"

#include "Aurora/Tools/ImGuiHelper.hpp"
#include "Aurora/Tools/ImGuizmo.h"

namespace Aurora
{
	struct RenderViewPort;

	class MainEditorPanel
	{
	private:
		std::shared_ptr<ConsoleWindow> m_ConsoleWindow;
		RenderViewPort* m_RenderViewPort;

		class Actor* m_SelectedActor;
		class SceneComponent* m_SelectedComponent;

		bool m_MouseViewportGrabbed;

		ImGuizmo::OPERATION m_CurrentManipulatorOperation;
		ImGuizmo::MODE m_CurrentManipulatorMode;

		bool m_IsPlayMode;

		float m_FlySpeed;
	public:
		MainEditorPanel();
		~MainEditorPanel();

		void Update(double delta);

		void BeginDockSpace();
		void DrawMainMenu();

		[[nodiscard]] bool IsPlayMode() const { return m_IsPlayMode; }
	};
}
