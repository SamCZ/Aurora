#pragma once

#include <utility>

#include "Aurora/Engine.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Tools/ImGuiHelper.hpp"
#include "Aurora/Tools/IconsFontAwesome5.hpp"

namespace Aurora
{
	class EditorWindowBase
	{
	protected:
		const char* m_WindowName;
		bool m_WindowOpened;
		bool m_WindowNeedsFocus;
	public:
		EditorWindowBase(const char* mName, bool mWindowOpened, bool mWindowNeedsFocus) : m_WindowName(mName), m_WindowOpened(mWindowOpened), m_WindowNeedsFocus(mWindowNeedsFocus) {}

		virtual void Update(double delta)
		{
			if (!m_WindowOpened)
			{
				m_WindowNeedsFocus = true;
				return;
			}

			if(m_WindowNeedsFocus)
			{
				ImGui::SetNextWindowFocus();
				m_WindowNeedsFocus = false;
			}

			if (ImGui::Begin(m_WindowName, &m_WindowOpened))
			{
				OnGui();
			}
			ImGui::End();
		}

		virtual void OnGui() = 0;

		void OpenWindow(bool focus)
		{
			m_WindowOpened = true;
			m_WindowNeedsFocus = focus;
		}

		void CloseWindow()
		{
			m_WindowOpened = false;
			m_WindowNeedsFocus = false;
		}
	};
}