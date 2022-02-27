#pragma once

#include "ConsoleWindow.hpp"

namespace Aurora
{
	class MainEditorPanel
	{
	private:
		std::shared_ptr<ConsoleWindow> m_ConsoleWindow;
	public:
		MainEditorPanel();

		void Update();

		void BeginDockSpace();
		void DrawMainMenu();
	};
}
