#include "Window.hpp"

#include <Aurora/Core/UTF8.hpp>

#if _WIN32
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "Input/GLFW/Manager.hpp"

namespace Aurora
{
	Window::Window()
			: m_WindowHandle(nullptr), m_Focused(false),
			  m_CursorMode(ECursorMode::Normal), m_InputManager(Input::Manager_ptr(new Input::Manager(this))),
			  /*m_SwapChain(nullptr), */m_Vsync(true)
	{

	}

	void Window::Initialize(const WindowDefinition& windowDefinition, const std::shared_ptr<Window>& parentWindow)
	{
		m_Title = windowDefinition.Title;

		if(/* is vulkan */true) {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		}

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
		//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

		if(!windowDefinition.HasOSWindowBorder) {
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		}

		if(windowDefinition.Maximized) {
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		}

		SetSize(windowDefinition.Width, windowDefinition.Height);

		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		const GLFWvidmode* vidMode = glfwGetVideoMode(primary);

		m_WindowHandle = glfwCreateWindow(GetSize().x, GetSize().y, windowDefinition.Title.c_str(), nullptr, nullptr);
		glfwSwapInterval(0);

		if(!m_WindowHandle) {
			std::cerr << "Cannot create window !" << std::endl;
			glfwTerminate();
			exit(1);
		}

		if(parentWindow != nullptr && !windowDefinition.Maximized) {
			// TODO: Center relative to parent
		} else if(!windowDefinition.Maximized) {
			int x = vidMode->width / 2 - windowDefinition.Width / 2;
			int y = vidMode->height / 2 - windowDefinition.Height / 2;
			glfwSetWindowPos(m_WindowHandle, x, y);
		}

		if(parentWindow != nullptr) {
#if _WIN32
			HWND hwNative = glfwGetWin32Window(m_WindowHandle);
			HWND parentHwNative = glfwGetWin32Window(parentWindow->GetWindowHandle());

			::SetParent(hwNative, parentHwNative);
#endif
		}

		if(false) { // If opengl render device
			glfwMakeContextCurrent(m_WindowHandle);

			// This is gonna break after second window is created !
			if(!gladLoadGL()) {
				printf("\n");
				exit(-1);
			}
			printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);

			if(!GLAD_GL_EXT_texture_array) {
				std::cerr << "GLAD_GL_EXT_texture_array not found !" << std::endl;
			}

			glfwSwapInterval(0);
		}
		glfwSetInputMode(m_WindowHandle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

		glfwSetWindowUserPointer(m_WindowHandle, this);

		glfwSetWindowSizeCallback(m_WindowHandle, Window::OnResizeCallback);
		glfwSetWindowFocusCallback(m_WindowHandle, Window::OnFocusCallback);

		glfwSetKeyCallback(m_WindowHandle, Window::OnKeyCallback);
		glfwSetCursorPosCallback(m_WindowHandle, Window::OnCursorPosCallBack);
		glfwSetScrollCallback(m_WindowHandle, Window::OnMouseScrollCallback);
		glfwSetMouseButtonCallback(m_WindowHandle, Window::OnMouseButtonCallback);

		glfwSetCharModsCallback(m_WindowHandle, Window::CharModsCallback);
	}

	void Window::Show()
	{
		if(m_WindowHandle != nullptr)
			glfwShowWindow(m_WindowHandle);
	}

	void Window::Hide()
	{
		if(m_WindowHandle != nullptr)
			glfwHideWindow(m_WindowHandle);
	}

	void Window::Destroy()
	{
		if(m_WindowHandle != nullptr)
			glfwDestroyWindow(m_WindowHandle);
	}

	void Window::Minimize()
	{
		if(m_WindowHandle != nullptr)
			glfwIconifyWindow(m_WindowHandle);
	}

	void Window::Maximize()
	{
		if(m_WindowHandle != nullptr)
			glfwMaximizeWindow(m_WindowHandle);
	}

	void Window::Restore()
	{
		if(m_WindowHandle != nullptr)
			glfwRestoreWindow(m_WindowHandle);
	}

	void Window::Focus()
	{
		if(m_WindowHandle != nullptr)
			glfwFocusWindow(m_WindowHandle);
	}

	void Window::SetTitle(const String& title)
	{
		glfwSetWindowTitle(m_WindowHandle, title.c_str());
	}

	GLFWwindow* Window::GetWindowHandle()
	{
		return m_WindowHandle;
	}

	bool Window::IsShouldClose() const
	{
		if(m_WindowHandle == nullptr)
			return false;

		return glfwWindowShouldClose(m_WindowHandle);
	}

	void Window::SetCursorMode(const ECursorMode& mode)
	{
		if(m_WindowHandle == nullptr)
			return;

		switch (mode)
		{
			case ECursorMode::Normal:
				glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				break;
			case ECursorMode::Disabled:
				glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				break;
			case ECursorMode::Hidden:
				glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				break;
		}

		m_CursorMode = mode;
	}

	const ECursorMode &Window::GetCursorMode() const
	{
		return m_CursorMode;
	}

	bool Window::IsFocused() const
	{
		return m_Focused;
	}

	bool Window::IsIconified()
	{
		return glfwGetWindowAttrib(m_WindowHandle, GLFW_ICONIFIED) == GLFW_TRUE;
	}

	Vector2i Window::GetCursorPos()
	{
		double x;
		double y;
		glfwGetCursorPos(m_WindowHandle, &x, &y);
		return Vector2i(static_cast<int>(x), static_cast<int>(y));
	}

	::Aurora::Input::IManager_ptr& Window::GetInputManager()
	{
		return m_InputManager;
	}

	/**
	 * Callbacks
	 */

	void Window::OnResizeCallback(GLFWwindow* rawWindow, int width, int height)
	{
		auto* window = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
		window->SetSize(width, height);

		if(window->GetSwapChain() != nullptr) {
			window->GetSwapChain()->Resize(width, height);
		}
	}

	void Window::OnFocusCallback(GLFWwindow *rawWindow, int focused)
	{
		auto* window = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
		window->m_Focused = focused;
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnFocusChange(focused != GLFW_FALSE);

		//TODO: Call callbacks
	}

	void Window::OnKeyCallback(GLFWwindow *rawWindow, int key, int scancode, int action, int mods)
	{
		bool pressed;
		switch(action)
		{
			case GLFW_PRESS:
				pressed = true;
				break;
			case GLFW_RELEASE:
				pressed = false;
				break;
			case GLFW_REPEAT:
				return; // Not usable
			default:
				return; // Unknown key action
		}

		auto* window = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnKeyChange(key, scancode, pressed);
	}

	void Window::OnCursorPosCallBack(GLFWwindow *rawWindow, double xpos, double ypos)
	{
		glm::dvec2 newPosition = {xpos, ypos};

		auto* window = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnMouseMove(newPosition);
	}

	void Window::OnMouseScrollCallback(GLFWwindow *rawWindow, double xoffset, double yoffset)
	{
		auto* window = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnMouseWheel({xoffset, yoffset});
	}

	void Window::OnMouseButtonCallback(GLFWwindow *rawWindow, int button, int action, int mods)
	{
		bool pressed;
		switch(action)
		{
			case GLFW_PRESS:
				pressed = true;
				break;
			case GLFW_RELEASE:
				pressed = false;
				break;
			case GLFW_REPEAT:
				return; // Not usable
			default:
				return; // Unknown key action
		}

		auto* window = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnMouseButton(button, pressed);
	}

	void Window::CharModsCallback(GLFWwindow *rawWindow, uint32_t codepoint, int mods)
	{
		auto* window = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnTextInput(CodepointToUtf8(codepoint));
	}
}