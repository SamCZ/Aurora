#pragma once

#include <memory>
#include <string>

#include <GLFW/glfw3.h>

#include <SwapChain.h>
#include <RefCntAutoPtr.hpp>

#include "Aurora/Core/Common.hpp"
#include "Aurora/Core/Vector.hpp"

#include "CursorMode.hpp"

#include "Input/IManager.hpp"

using namespace Diligent;

namespace Aurora
{
	struct WindowDefinition
	{
		int Width;
		int Height;

		bool HasOSWindowBorder;
		bool Maximized;

		String Title;
	};

	AU_CLASS(Window)
	{
		friend class Input::IManager;
	private:
		String m_Title;
		GLFWwindow* m_WindowHandle;
		bool m_Focused;
		ECursorMode m_CursorMode;
		bool m_Vsync;
		Vector2i m_Size{};
	private:
		Input::IManager_ptr m_InputManager;
		RefCntAutoPtr<ISwapChain> m_SwapChain;
		std::vector<std::function<void(int, int)>> m_ResizeListeners;
	public:
		Window();
		~Window() = default;

		void Initialize(const WindowDefinition& windowDefinition, const Window_ptr& parentWindow);

		inline void SetSize(int width, int height)
		{
			m_Size.x = width;
			m_Size.y = height;
		}

		inline void SetSize(const Vector2i& size)
		{
			m_Size = size;
		}

		[[nodiscard]] inline const Vector2i& GetSize() const
		{
			return m_Size;
		}

		[[nodiscard]] inline int GetWidth() const
		{
			return m_Size.x;
		}

		[[nodiscard]] inline int GetHeight() const
		{
			return m_Size.y;
		}

		inline void AddResizeListener(const std::function<void(int, int)>& listener)
		{
			m_ResizeListeners.push_back(listener);
		}

		void Show();
		void Hide();

		void Destroy();

		void Minimize();
		void Maximize();
		void Restore();

		void Focus();

		void SetTitle(const String& title);

		[[nodiscard]] inline String GetOriginalTitle() const
		{
			return m_Title;
		}

		inline void SetVsync(bool enabled)
		{
			m_Vsync = enabled;
		}

		[[nodiscard]] inline bool IsVsyncEnabled() const
		{
			return m_Vsync;
		}

		[[nodiscard]] bool IsFocused() const;

		[[nodiscard]] bool IsShouldClose() const;

		virtual GLFWwindow* GetWindowHandle();

		void SetCursorMode(const ECursorMode& mode);
		[[nodiscard]] const ECursorMode& GetCursorMode() const;

		bool IsIconified();

		::Aurora::Input::IManager_ptr& GetInputManager();

		Vector2i GetCursorPos();
	public:
		inline void SetSwapChain(RefCntAutoPtr<ISwapChain> swapChain)
		{
			if(m_SwapChain != nullptr) return;

			m_SwapChain = std::move(swapChain);
		}

		inline RefCntAutoPtr<ISwapChain>& GetSwapChain()
		{
			return m_SwapChain;
		}
	private:
		static void OnResizeCallback(GLFWwindow* rawWindow,int width,int height);
		static void OnFocusCallback(GLFWwindow* rawWindow, int focused);
		static void OnKeyCallback(GLFWwindow* rawWindow, int key, int scancode, int action, int mods);
		static void OnCursorPosCallBack(GLFWwindow* rawWindow, double x, double y);
		static void OnMouseScrollCallback(GLFWwindow* rawWindow, double xOffset, double yOffset);
		static void OnMouseButtonCallback(GLFWwindow* rawWindow, int button, int action, int mods);
		static void CharModsCallback(GLFWwindow* rawWindow, uint32_t codepoint, int mods);
	};
}
