#pragma once

#include "ConsoleWindow.hpp"

namespace Aurora
{
	struct RenderViewPort;

	class MainEditorPanel
	{
	private:
		std::shared_ptr<ConsoleWindow> m_ConsoleWindow;
		RenderViewPort* m_RenderViewPort;

		class Actor* m_SelectedActor;
	public:
		MainEditorPanel();
		~MainEditorPanel();

		void Update();

		void BeginDockSpace();
		void DrawMainMenu();
	};
}
