#pragma once

#include <memory>
#include <thread>
#include "Window.hpp"

namespace Aurora
{
	AU_CLASS(WindowGameContext)
	{
	public:
		friend class AuroraEngine;
	private:
		std::shared_ptr<Window> m_Window;
		Input::IManager_ptr m_InputManager;
		std::thread::id m_ThreadId;
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

		inline ISwapChain_ptr& GetSwapChain()
		{
			return GetWindow()->GetSwapChain();
		}
	};
}
