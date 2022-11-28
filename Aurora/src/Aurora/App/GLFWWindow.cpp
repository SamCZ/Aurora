 #include "GLFWWindow.hpp"

#include <Aurora/Core/UTF8.hpp>

#if GLFW_ENABLED

#if _WIN32
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "Aurora/Engine.hpp"
#include "Input/GLFW/Manager.hpp"
#include "Aurora/Graphics/OpenGL/GL.hpp"
#include "Aurora/Graphics/OpenGL/GLUtils.hpp"


#include <Aurora/Engine.hpp>
#include <Aurora/RmlUI/RmlUI.hpp>
#include <Aurora/Graphics/ViewPortManager.hpp>

#include "Aurora/RmlUI/RmlMappings.hpp"

#define RML_UI_ENABLED 1
namespace Aurora
{
	double GetTimeInSeconds()
	{
		return glfwGetTime();
	}

	GLFWWindow::GLFWWindow() : ISystemWindow(), m_WindowHandle(nullptr), m_Focused(false),
							   m_CursorMode(ECursorMode::Normal), m_Vsync(true),
							   m_InputManager(Input::Manager_ptr(new Input::Manager(this))), m_SwapChain(nullptr)
	{

	}

	GLFWWindow::~GLFWWindow() = default;

	void GLFWWindow::Initialize(const WindowDefinition& windowDefinition, const std::shared_ptr<ISystemWindow>& parentWindow)
	{
		m_Title = windowDefinition.Title;

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		//glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		//glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);

		//glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);


		//glfwWindowHint(GLFW_DEPTH_BITS, 32);
		//glfwWindowHint(GLFW_STENCIL_BITS, 0);

		//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
		//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

		if(!windowDefinition.HasOSWindowBorder) {
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		}

		if(windowDefinition.Maximized && !windowDefinition.FullScreen) {
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		//glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_FALSE);

		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		const GLFWvidmode* vidMode = glfwGetVideoMode(primary);

		if (windowDefinition.FullScreen)
		{
			SetSize(vidMode->width, vidMode->height);
		}
		else
		{
			SetSize(windowDefinition.Width, windowDefinition.Height);
		}

		m_WindowHandle = glfwCreateWindow(GetSize().x, GetSize().y, windowDefinition.Title.c_str(), windowDefinition.FullScreen ? primary : nullptr, nullptr);

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
		} else if(!windowDefinition.Maximized && !windowDefinition.FullScreen) {
			int x = vidMode->width / 2 - GetWidth() / 2;
			int y = vidMode->height / 2 - GetHeight() / 2;
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

		glfwSetDropCallback(m_WindowHandle, GLFWWindow::OnFileDropListener);

		glfwSetJoystickCallback(GLFWWindow::JoystickCallback);

		//glDisable(GL_FRAMEBUFFER_SRGB);

		Focus();
	}

#if _WIN32
	 HWND GLFWWindow::GetWindowWin32Handle()
	 {
		 return glfwGetWin32Window(m_WindowHandle);
	 }
#endif

	void GLFWWindow::Show()
	{
		if(m_WindowHandle != nullptr)
			glfwShowWindow(m_WindowHandle);

		glfwSwapBuffers(m_WindowHandle);
		glfwWaitEvents();

#if UNIX
		// If on linux you startup window maximized, it will not call the callback and instead of set wrong size, this should fix that.
		int width;
		int height;
		glfwGetWindowSize(m_WindowHandle, &width, &height);

		if(m_Size.x != width || m_Size.y != height)
		{
			OnResizeCallback(m_WindowHandle, width, height);
		}
#endif
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
		return {static_cast<int>(x), static_cast<int>(y)};
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

		return {};
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
#ifdef RML_UI_ENABLED
	int ModifiersGLFWToRml(int modifier)
	{
		int rmlModifiers = 0;
		if (modifier & GLFW_MOD_ALT)
			rmlModifiers |= Rml::Input::KeyModifier::KM_ALT;
		if (modifier & GLFW_MOD_CONTROL)
			rmlModifiers |= Rml::Input::KeyModifier::KM_CTRL;
		if (modifier & GLFW_MOD_SHIFT)
			rmlModifiers |= Rml::Input::KeyModifier::KM_SHIFT;
		return rmlModifiers;
	}
#endif
	int currentMods = 0;

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
#ifdef RML_UI_ENABLED
		// Rml
		if(GEngine->GetRmlUI() == nullptr || GEngine->GetRmlUI()->GetRmlContext() == nullptr) return;

		currentMods = mods;

		auto rmlkey = RmlKeyMap.find(key);

		if(rmlkey == RmlKeyMap.end()) return;

		if(pressed)
		{
			auto rmlKeyId = (Rml::Input::KeyIdentifier)rmlkey->second;
			GEngine->GetRmlUI()->GetRmlContext()->ProcessKeyDown(rmlKeyId, ModifiersGLFWToRml(mods));

			if (rmlKeyId == Rml::Input::KI_RETURN || rmlKeyId == Rml::Input::KI_NUMPADENTER)
				GEngine->GetRmlUI()->GetRmlContext()->ProcessTextInput('\n');
		} else {
			GEngine->GetRmlUI()->GetRmlContext()->ProcessKeyUp((Rml::Input::KeyIdentifier)rmlkey->second, ModifiersGLFWToRml(mods));
		}
#endif
	}

	void GLFWWindow::OnCursorPosCallBack(GLFWwindow *rawWindow, double xpos, double ypos)
	{
		glm::dvec2 newPosition = {xpos, ypos};

		auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnMouseMove(newPosition);
#ifdef RML_UI_ENABLED
		// Rml

		if(GEngine->GetRmlUI() == nullptr || GEngine->GetRmlUI()->GetRmlContext() == nullptr) return;

		if(RenderViewPort* wp = GEngine->GetViewPortManager()->Get())
		{
			Vector2i proxyLocation = wp->ProxyLocation;

			Vector2i viewportSize;
			viewportSize.x = wp->ViewPort.Width;
			viewportSize.y = wp->ViewPort.Height;

			int cursorX = static_cast<int>((xpos)) - proxyLocation.x;
			int cursorY = static_cast<int>((ypos)) - proxyLocation.y;

			if(cursorX >= 0 && cursorY >= 0)
			{
				GEngine->GetRmlUI()->GetRmlContext()->ProcessMouseMove(cursorX, cursorY, ModifiersGLFWToRml(currentMods));
			}
		}
		else
		{
			GEngine->GetRmlUI()->GetRmlContext()->ProcessMouseMove(static_cast<int>(round(xpos)), static_cast<int>(round(ypos)), ModifiersGLFWToRml(currentMods));
		}
#endif
	}

	void GLFWWindow::OnMouseScrollCallback(GLFWwindow *rawWindow, double xoffset, double yoffset)
	{
		auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnMouseWheel({xoffset, yoffset});
#ifdef RML_UI_ENABLED
		// Rml
		if(GEngine->GetRmlUI() == nullptr || GEngine->GetRmlUI()->GetRmlContext() == nullptr) return;
		GEngine->GetRmlUI()->GetRmlContext()->ProcessMouseWheel(-yoffset, ModifiersGLFWToRml(currentMods));
#endif
	}

	int MouseButtonGLFWToRml(int button)
	{
		int rmlButton = -1;
		switch (button)
		{
			case GLFW_MOUSE_BUTTON_LEFT:   rmlButton = 0; break; // Left
			case GLFW_MOUSE_BUTTON_MIDDLE: rmlButton = 2; break; // Middle
			case GLFW_MOUSE_BUTTON_RIGHT:  rmlButton = 1; break; // Right
			case GLFW_MOUSE_BUTTON_4:     rmlButton = 3; break; // X1
			case GLFW_MOUSE_BUTTON_5:     rmlButton = 4; break; // X2
			default:                           break;
		}
		return rmlButton;
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
#ifdef RML_UI_ENABLED
		// Rml
		if(GEngine->GetRmlUI() == nullptr || GEngine->GetRmlUI()->GetRmlContext() == nullptr) return;
		if(pressed)
		{
			GEngine->GetRmlUI()->GetRmlContext()->ProcessMouseButtonDown(MouseButtonGLFWToRml(button), ModifiersGLFWToRml(currentMods));
		} else {
			GEngine->GetRmlUI()->GetRmlContext()->ProcessMouseButtonUp(MouseButtonGLFWToRml(button), ModifiersGLFWToRml(currentMods));
		}
#endif
	}

	void GLFWWindow::CharModsCallback(GLFWwindow *rawWindow, uint32_t codepoint, int mods)
	{
		auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));
		auto c = CodepointToUtf8(codepoint);
		std::dynamic_pointer_cast<Input::Manager>(window->GetInputManager())->OnTextInput(c);
#ifdef RML_UI_ENABLED
		// Rml
		if(GEngine->GetRmlUI() == nullptr || GEngine->GetRmlUI()->GetRmlContext() == nullptr) return;

		std::string str;
		for (const auto &item : c) str += (char)item;
		GEngine->GetRmlUI()->GetRmlContext()->ProcessTextInput(str);
#endif
	}

	 void GLFWWindow::OnFileDropListener(GLFWwindow* rawWindow, int count, const char** paths)
	 {
		if (!count)
		{
			return;
		}

		 auto* window = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(rawWindow));

		std::vector<Path> files;

		 for (int i = 0; i < count; ++i)
		 {
			 files.emplace_back(paths[i]);
		 }

		 window->m_DropFileEmitter.Invoke(std::forward<std::vector<Path>>(files));
	 }

	 void GLFWWindow::JoystickCallback(int jid, int event)
	 {
		 std::dynamic_pointer_cast<Input::Manager>(GEngine->GetWindow()->GetInputManager())->OnJoystickConnectChange(jid, event == GLFW_CONNECTED);

		 if (event == GLFW_CONNECTED)
		 {
			 AU_LOG_INFO("The joystick was connected ", jid);
		 }
		 else if (event == GLFW_DISCONNECTED)
		 {
			 AU_LOG_INFO("The joystick was disconnected ", jid);
		 }
	 }
 }
#endif