#pragma once

#include "ConsoleWindow.hpp"

#include "Aurora/Graphics/Base/Texture.hpp"

namespace Aurora
{
	struct RenderViewPort;

	class MainEditorPanel
	{
	private:
		std::shared_ptr<ConsoleWindow> m_ConsoleWindow;
		RenderViewPort* m_RenderViewPort;

		Texture_ptr m_FolderTexture;
		Texture_ptr m_FileTexture;

		class Actor* m_SelectedActor;
		class SceneComponent* m_SelectedComponent;

		bool m_MouseViewportGrabbed;
	public:
		MainEditorPanel();
		~MainEditorPanel();

		void Update(double delta);

		void BeginDockSpace();
		void DrawMainMenu();
	};
}
