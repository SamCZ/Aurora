#pragma once

#if GLFW_ENABLED

#if _WIN32
#include <Windows.h>
#endif

#include "IWindow.hpp"
#include <GLFW/glfw3.h>

namespace Aurora
{
	AU_CLASS(GLFWWindow) : public IWindow
	{
		friend class Input::IManager;
	private:
		String m_Title;
#if GLFW_ENABLED
		GLFWwindow* m_WindowHandle;
#endif
		bool m_Focused;
		ECursorMode m_CursorMode;
		bool m_Vsync;
		Vector2i m_Size{};
	private:
		Input::IManager_ptr m_InputManager;
		ISwapChain_ptr m_SwapChain;
		std::vector<std::function<void(int, int)>> m_ResizeListeners;
	public:
		GLFWWindow();
		~GLFWWindow() override;

		void Initialize(const WindowDefinition& windowDefinition, const IWindow_ptr& parentWindow) override;

		inline GLFWwindow* GetHandle()
		{
			return m_WindowHandle;
		}

		inline void SetSize(int width, int height) override
		{
			m_Size.x = width;
			m_Size.y = height;
		}

		inline void SetSize(const Vector2i& size) override
		{
			m_Size = size;
		}

		[[nodiscard]] inline const Vector2i& GetSize() const override
		{
			return m_Size;
		}

		[[nodiscard]] inline int GetWidth() const override
		{
			return m_Size.x;
		}

		[[nodiscard]] inline int GetHeight() const override
		{
			return m_Size.y;
		}

		inline void AddResizeListener(const std::function<void(int, int)>& listener) override
		{
			m_ResizeListeners.push_back(listener);
		}

		void Show() override;
		void Hide() override;

		void Destroy() override;

		void Minimize() override;
		void Maximize() override;
		void Restore() override;

		void Focus() override;

		void SetTitle(const String& title) override;

		[[nodiscard]] inline const String& GetOriginalTitle() const override
		{
			return m_Title;
		}

		inline void SetVsync(bool enabled) override
		{
			m_Vsync = enabled;
		}

		[[nodiscard]] inline bool IsVsyncEnabled() const override
		{
			return m_Vsync;
		}

		[[nodiscard]] bool IsFocused() const override;

		[[nodiscard]] bool IsShouldClose() const override;
#if GLFW_ENABLED
		virtual GLFWwindow* GetWindowHandle();
#endif
		void SetCursorMode(const ECursorMode& mode) override;
		[[nodiscard]] const ECursorMode& GetCursorMode() const override;

		bool IsIconified() override;

		::Aurora::Input::IManager_ptr& GetInputManager() override;

		Vector2i GetCursorPos();

		void SetClipboardString(const String& str) override;
		String GetClipboardString() override;
	public:
		inline void SetSwapChain(ISwapChain_ptr swapChain) override
		{
			if(m_SwapChain != nullptr) return;

			m_SwapChain = std::move(swapChain);
		}

		inline ISwapChain_ptr& GetSwapChain() override
		{
			return m_SwapChain;
		}
	private:
#if GLFW_ENABLED
		static void OnResizeCallback(GLFWwindow* rawWindow,int width,int height);
		static void OnFocusCallback(GLFWwindow* rawWindow, int focused);
		static void OnKeyCallback(GLFWwindow* rawWindow, int key, int scancode, int action, int mods);
		static void OnCursorPosCallBack(GLFWwindow* rawWindow, double x, double y);
		static void OnMouseScrollCallback(GLFWwindow* rawWindow, double xOffset, double yOffset);
		static void OnMouseButtonCallback(GLFWwindow* rawWindow, int button, int action, int mods);
		static void CharModsCallback(GLFWwindow* rawWindow, uint32_t codepoint, int mods);
#endif
	};
}
#endif