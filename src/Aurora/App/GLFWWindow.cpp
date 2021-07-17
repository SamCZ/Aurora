#include "GLFWWindow.hpp"

#include <Aurora/Core/UTF8.hpp>

#if GLFW_ENABLED

#if _WIN32
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "Input/GLFW/Manager.hpp"

#include <glad/glad.h>

namespace Aurora
{
	GLFWWindow::GLFWWindow() : IWindow(), m_WindowHandle(nullptr), m_Focused(false),
							   m_CursorMode(ECursorMode::Normal), m_InputManager(Input::Manager_ptr(new Input::Manager(this))),
							   m_SwapChain(nullptr), m_Vsync(true)
	{

	}

	void GLAPIENTRY
	MessageCallback( GLenum source,
					 GLenum type,
					 GLuint id,
					 GLenum severity,
					 GLsizei length,
					 const GLchar* message,
					 const void* userParam )
	{
		// Note: disabling flood of notifications through glDebugMessageControl() has no effect,
		// so we have to filter them out here
		if (id == 131185 || // Buffer detailed info: Buffer object <X> (bound to GL_XXXX ... , usage hint is GL_DYNAMIC_DRAW)
			// will use VIDEO memory as the source for buffer object operations.
			id == 131186 ||   // Buffer object <X> (bound to GL_XXXX, usage hint is GL_DYNAMIC_DRAW) is being copied/moved from VIDEO memory to HOST memory.
			id == 131204 // Unused texture
				)
			return;

		std::stringstream MessageSS;

		MessageSS << "OpenGL debug message " << id << " (";
		switch (source)
		{
			// clang-format off
			case GL_DEBUG_SOURCE_API:             MessageSS << "Source: API.";             break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   MessageSS << "Source: Window System.";   break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: MessageSS << "Source: Shader Compiler."; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     MessageSS << "Source: Third Party.";     break;
			case GL_DEBUG_SOURCE_APPLICATION:     MessageSS << "Source: Application.";     break;
			case GL_DEBUG_SOURCE_OTHER:           MessageSS << "Source: Other.";           break;
			default:                              MessageSS << "Source: Unknown (" << source << ").";
				// clang-format on
		}

		switch (type)
		{
			// clang-format off
			case GL_DEBUG_TYPE_ERROR:               MessageSS << " Type: ERROR.";                break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: MessageSS << " Type: Deprecated Behaviour."; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  MessageSS << " Type: UNDEFINED BEHAVIOUR.";  break;
			case GL_DEBUG_TYPE_PORTABILITY:         MessageSS << " Type: Portability.";          break;
			case GL_DEBUG_TYPE_PERFORMANCE:         MessageSS << " Type: PERFORMANCE.";          break;
			case GL_DEBUG_TYPE_MARKER:              MessageSS << " Type: Marker.";               break;
			case GL_DEBUG_TYPE_PUSH_GROUP:          MessageSS << " Type: Push Group.";           break;
			case GL_DEBUG_TYPE_POP_GROUP:           MessageSS << " Type: Pop Group.";            break;
			case GL_DEBUG_TYPE_OTHER:               MessageSS << " Type: Other.";                break;
			default:                                MessageSS << " Type: Unknown (" << type << ").";
				// clang-format on
		}

		switch (severity)
		{
			// clang-format off
			case GL_DEBUG_SEVERITY_HIGH:         MessageSS << " Severity: HIGH";         break;
			case GL_DEBUG_SEVERITY_MEDIUM:       MessageSS << " Severity: Medium";       break;
			case GL_DEBUG_SEVERITY_LOW:          MessageSS << " Severity: Low";          break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: MessageSS << " Severity: Notification"; break;
			default:                             MessageSS << " Severity: Unknown (" << severity << ")"; break;
				// clang-format on
		}

		MessageSS << "): " << message;

		AU_LOG_INFO(MessageSS.str());
	}

	void GLFWWindow::Initialize(const WindowDefinition& windowDefinition, const std::shared_ptr<IWindow>& parentWindow)
	{
		m_Title = windowDefinition.Title;

		if(/* is vulkan */true) {
			//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			//glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		}
#if GLFW_ENABLED
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);

		//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
		//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

		if(!windowDefinition.HasOSWindowBorder) {
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		}

		if(windowDefinition.Maximized) {
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		const GLFWvidmode* vidMode = glfwGetVideoMode(primary);
#endif
		SetSize(windowDefinition.Width, windowDefinition.Height);
		m_WindowHandle = glfwCreateWindow(GetSize().x, GetSize().y, windowDefinition.Title.c_str(), nullptr, nullptr);

		int width, height;
		glfwGetWindowSize(m_WindowHandle, &width, &height);

		SetSize(width, height);

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
			HWND parentHwNative = glfwGetWin32Window(((GLFWWindow*)parentWindow.get())->GetWindowHandle());

			::SetParent(hwNative, parentHwNative);
#endif
		}

		// Init gl
		glfwMakeContextCurrent(m_WindowHandle);

		// This is gonna break after second window is created !
		if(!gladLoadGL()) {
			printf("Something went wrong!\n");
			exit(-1);
		}
		// During init, enable debug output
		//glEnable              ( GL_DEBUG_OUTPUT );
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback( MessageCallback, 0 );
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

		AU_LOG_INFO("OpenGL ", GLVersion.major, ".", GLVersion.minor)

		if(!GLAD_GL_EXT_texture_array) {
			AU_LOG_ERROR("GLAD_GL_EXT_texture_array not found !");
		}

		// Handling

		glfwSetInputMode(m_WindowHandle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

		glfwSetWindowUserPointer(m_WindowHandle, this);

		glfwSetWindowSizeCallback(m_WindowHandle, GLFWWindow::OnResizeCallback);
		glfwSetWindowFocusCallback(m_WindowHandle, GLFWWindow::OnFocusCallback);

		glfwSetKeyCallback(m_WindowHandle, GLFWWindow::OnKeyCallback);
		glfwSetCursorPosCallback(m_WindowHandle, GLFWWindow::OnCursorPosCallBack);
		glfwSetScrollCallback(m_WindowHandle, GLFWWindow::OnMouseScrollCallback);
		glfwSetMouseButtonCallback(m_WindowHandle, GLFWWindow::OnMouseButtonCallback);

		glfwSetCharModsCallback(m_WindowHandle, GLFWWindow::CharModsCallback);




	}

	void GLFWWindow::Show()
	{
		if(m_WindowHandle != nullptr)
			glfwShowWindow(m_WindowHandle);

		glfwSwapBuffers(m_WindowHandle);
		glfwWaitEvents();
	}

	void GLFWWindow::Hide()
	{
		if(m_WindowHandle != nullptr)
			glfwHideWindow(m_WindowHandle);
	}

	void GLFWWindow::Destroy()
	{
		if(m_WindowHandle != nullptr)
			glfwDestroyWindow(m_WindowHandle);
	}

	void GLFWWindow::Minimize()
	{
		if(m_WindowHandle != nullptr)
			glfwIconifyWindow(m_WindowHandle);
	}

	void GLFWWindow::Maximize()
	{
		if(m_WindowHandle != nullptr)
			glfwMaximizeWindow(m_WindowHandle);
	}

	void GLFWWindow::Restore()
	{
		if(m_WindowHandle != nullptr)
			glfwRestoreWindow(m_WindowHandle);
	}

	void GLFWWindow::Focus()
	{
		if(m_WindowHandle != nullptr)
			glfwFocusWindow(m_WindowHandle);
	}

	void GLFWWindow::SetTitle(const String& title)
	{
		glfwSetWindowTitle(m_WindowHandle, title.c_str());
	}

	GLFWwindow* GLFWWindow::GetWindowHandle()
	{
		return m_WindowHandle;
	}

	bool GLFWWindow::IsShouldClose() const
	{
		if(m_WindowHandle == nullptr)
			return false;

		return glfwWindowShouldClose(m_WindowHandle);
	}

	void GLFWWindow::SetCursorMode(const ECursorMode& mode)
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

	const ECursorMode &GLFWWindow::GetCursorMode() const
	{
		return m_CursorMode;
	}

	bool GLFWWindow::IsFocused() const
	{
		return m_Focused;
	}

	bool GLFWWindow::IsIconified()
	{
		return glfwGetWindowAttrib(m_WindowHandle, GLFW_ICONIFIED) == GLFW_TRUE;
	}

	Vector2i GLFWWindow::GetCursorPos()
	{
		double x;
		double y;
		glfwGetCursorPos(m_WindowHandle, &x, &y);
		return Vector2i(static_cast<int>(x), static_cast<int>(y));
	}

	::Aurora::Input::IManager_ptr& GLFWWindow::GetInputManager()
	{
		return m_InputManager;
	}

	void Aurora::GLFWWindow::SetClipboardString(const String &str)
	{
		glfwSetClipboardString(m_WindowHandle, str.c_str());
	}

	String GLFWWindow::GetClipboardString()
	{
		auto clipboardString = glfwGetClipboardString(m_WindowHandle);
		if(strlen(clipboardString) > 0) {
			return clipboardString;
		}

		return String();
	}

	/**
	 * Callbacks
	 */

	void GLFWWindow::OnResizeCallback(GLFWwindow* rawWindow, int width, int height)
	{
		if(width == 0 || height == 0) return;

		auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));
		window->SetSize(width, height);

		if(window->GetSwapChain() != nullptr) {
			window->GetSwapChain()->Resize(width, height);
		}

		for(auto& listener : window->m_ResizeListeners) {
			if(listener) listener(width, height);
		}
	}

	void GLFWWindow::OnFocusCallback(GLFWwindow *rawWindow, int focused)
	{
		auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));
		window->m_Focused = focused;
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnFocusChange(focused != GLFW_FALSE);

		//TODO: Call callbacks
	}

	void GLFWWindow::OnKeyCallback(GLFWwindow *rawWindow, int key, int scancode, int action, int mods)
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

		auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnKeyChange(key, scancode, pressed);
	}

	void GLFWWindow::OnCursorPosCallBack(GLFWwindow *rawWindow, double xpos, double ypos)
	{
		glm::dvec2 newPosition = {xpos, ypos};

		auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnMouseMove(newPosition);
	}

	void GLFWWindow::OnMouseScrollCallback(GLFWwindow *rawWindow, double xoffset, double yoffset)
	{
		auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnMouseWheel({xoffset, yoffset});
	}

	void GLFWWindow::OnMouseButtonCallback(GLFWwindow *rawWindow, int button, int action, int mods)
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

		auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnMouseButton(button, pressed);
	}

	void GLFWWindow::CharModsCallback(GLFWwindow *rawWindow, uint32_t codepoint, int mods)
	{
		auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnTextInput(CodepointToUtf8(codepoint));
	}
}
#endif