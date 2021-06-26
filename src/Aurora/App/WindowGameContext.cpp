#include "WindowGameContext.hpp"

namespace Aurora
{
	WindowGameContext::WindowGameContext(IWindow_ptr window) : m_Window(std::move(window)), m_InputManager(m_Window->GetInputManager())
	{
		m_Window->AddResizeListener([this](int width, int height) -> void {
			Resize(width, height);
		});
	}

	const IWindow_ptr& WindowGameContext::GetWindow()
	{
		return m_Window;
	}

	const Input::IManager_ptr &WindowGameContext::GetInputManager()
	{
		return m_InputManager;
	}
}