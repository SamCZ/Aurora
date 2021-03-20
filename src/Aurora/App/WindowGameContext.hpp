#pragma once

#include <memory>
#include "Window.hpp"

namespace Aurora
{
	AU_CLASS(WindowGameContext)
	{
	private:
		std::shared_ptr<Window> m_Window;
		Input::IManager_ptr m_InputManager;
	public:
		explicit WindowGameContext(Window_ptr window);
		virtual ~WindowGameContext() = default;

		virtual void Init() {}
		virtual void Update(double delta, double currentTime) {}
		virtual void Render() {}

		virtual void Resize(int width, int height) {}

	public:
		const Window_ptr& GetWindow();
		const Input::IManager_ptr& GetInputManager();

		inline RefCntAutoPtr<ISwapChain>& GetSwapChain()
		{
			return GetWindow()->GetSwapChain();
		}
	};
}
