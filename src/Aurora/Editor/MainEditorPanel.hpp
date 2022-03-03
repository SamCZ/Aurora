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
	public:
		MainEditorPanel();
		~MainEditorPanel();

		void Update();

		void BeginDockSpace();
		void DrawMainMenu();
	};
}
