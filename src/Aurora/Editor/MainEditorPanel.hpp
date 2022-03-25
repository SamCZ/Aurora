#pragma once

#include "ConsoleWindow.hpp"
#include "GameViewportPanel.hpp"

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Graphics/Base/Texture.hpp"

namespace Aurora
{
	struct RenderViewPort;

	class MainEditorPanel
	{
	private:
		std::shared_ptr<ConsoleWindow> m_ConsoleWindow;
		std::shared_ptr<GameViewportPanel> m_GameViewportWindow;

		class Actor* m_SelectedActor;
		class SceneComponent* m_SelectedComponent;

		bool m_IsPlayMode;
	public:
		MainEditorPanel();
		~MainEditorPanel();

		void Update(double delta);

		void BeginDockSpace();
		void DrawMainMenu();

		[[nodiscard]] bool IsPlayMode() const { return m_IsPlayMode; }
		void SetPlayMode(bool playMode) { m_IsPlayMode = playMode; }

		[[nodiscard]] inline bool IsAnySceneObjectSelected() const { return m_SelectedActor || m_SelectedComponent; }
		[[nodiscard]] bool GetSelectedObjectTransform(Matrix4& matrix) const;
		[[nodiscard]] bool SetSelectedObjectTransform(const Matrix4& matrix);
	};
}
