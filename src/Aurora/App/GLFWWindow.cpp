 #include "GLFWWindow.hpp"

#include <Aurora/Core/UTF8.hpp>

#if GLFW_ENABLED

#if _WIN32
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "Input/GLFW/Manager.hpp"
#include "Aurora/Graphics/OpenGL/GL.hpp"


#include <Aurora/Engine.hpp>
#include <Aurora/RmlUI/RmlUI.hpp>
#include <Aurora/Graphics/ViewPortManager.hpp>
#define RML_UI_ENABLED 1

namespace Aurora
{
	double GetTimeInSeconds()
	{
		return glfwGetTime();
	}

#ifdef RML_UI_ENABLED
	static const std::unordered_map<unsigned, uint16_t> RmlKeyMap {
			{ GLFW_KEY_SPACE, Rml::Input::KI_SPACE },
			{ GLFW_KEY_0, Rml::Input::KI_0 },
			{ GLFW_KEY_1, Rml::Input::KI_1 },
			{ GLFW_KEY_2, Rml::Input::KI_2 },
			{ GLFW_KEY_3, Rml::Input::KI_3 },
			{ GLFW_KEY_4, Rml::Input::KI_4 },
			{ GLFW_KEY_5, Rml::Input::KI_5 },
			{ GLFW_KEY_6, Rml::Input::KI_6 },
			{ GLFW_KEY_7, Rml::Input::KI_7 },
			{ GLFW_KEY_8, Rml::Input::KI_8 },
			{ GLFW_KEY_9, Rml::Input::KI_9 },
			{ GLFW_KEY_A, Rml::Input::KI_A },
			{ GLFW_KEY_B, Rml::Input::KI_B },
			{ GLFW_KEY_C, Rml::Input::KI_C },
			{ GLFW_KEY_D, Rml::Input::KI_D },
			{ GLFW_KEY_E, Rml::Input::KI_E },
			{ GLFW_KEY_F, Rml::Input::KI_F },
			{ GLFW_KEY_G, Rml::Input::KI_G },
			{ GLFW_KEY_H, Rml::Input::KI_H },
			{ GLFW_KEY_I, Rml::Input::KI_I },
			{ GLFW_KEY_J, Rml::Input::KI_J },
			{ GLFW_KEY_K, Rml::Input::KI_K },
			{ GLFW_KEY_L, Rml::Input::KI_L },
			{ GLFW_KEY_M, Rml::Input::KI_M },
			{ GLFW_KEY_N, Rml::Input::KI_N },
			{ GLFW_KEY_O, Rml::Input::KI_O },
			{ GLFW_KEY_P, Rml::Input::KI_P },
			{ GLFW_KEY_Q, Rml::Input::KI_Q },
			{ GLFW_KEY_R, Rml::Input::KI_R },
			{ GLFW_KEY_S, Rml::Input::KI_S },
			{ GLFW_KEY_T, Rml::Input::KI_T },
			{ GLFW_KEY_U, Rml::Input::KI_U },
			{ GLFW_KEY_V, Rml::Input::KI_V },
			{ GLFW_KEY_W, Rml::Input::KI_W },
			{ GLFW_KEY_X, Rml::Input::KI_X },
			{ GLFW_KEY_Y, Rml::Input::KI_Y },
			{ GLFW_KEY_Z, Rml::Input::KI_Z },
			{ GLFW_KEY_SEMICOLON, Rml::Input::KI_OEM_1 },           // US standard keyboard; the ';:' key.
			{ GLFW_KEY_EQUAL, Rml::Input::KI_OEM_PLUS },           // Any region; the '=+' key.
			{ GLFW_KEY_COMMA, Rml::Input::KI_OEM_COMMA },           // Any region; the ',<' key.
			{ GLFW_KEY_MINUS, Rml::Input::KI_OEM_MINUS },           // Any region; the '-_' key.
			{ GLFW_KEY_PERIOD, Rml::Input::KI_OEM_PERIOD },         // Any region; the '.>' key.
			{ GLFW_KEY_SLASH, Rml::Input::KI_OEM_2 },               // Any region; the '/?' key.
			{ GLFW_KEY_LEFT_BRACKET, Rml::Input::KI_OEM_4 },         // US standard keyboard; the '[{' key.
			{ GLFW_KEY_BACKSLASH, Rml::Input::KI_OEM_5 },           // US standard keyboard; the '\|' key.
			{ GLFW_KEY_RIGHT_BRACKET, Rml::Input::KI_OEM_6 },        // US standard keyboard; the ']}' key.
			{ GLFW_KEY_KP_0, Rml::Input::KI_NUMPAD0 },
			{ GLFW_KEY_KP_1, Rml::Input::KI_NUMPAD1 },
			{ GLFW_KEY_KP_2, Rml::Input::KI_NUMPAD2 },
			{ GLFW_KEY_KP_3, Rml::Input::KI_NUMPAD3 },
			{ GLFW_KEY_KP_4, Rml::Input::KI_NUMPAD4 },
			{ GLFW_KEY_KP_5, Rml::Input::KI_NUMPAD5 },
			{ GLFW_KEY_KP_6, Rml::Input::KI_NUMPAD6 },
			{ GLFW_KEY_KP_7, Rml::Input::KI_NUMPAD7 },
			{ GLFW_KEY_KP_8, Rml::Input::KI_NUMPAD8 },
			{ GLFW_KEY_KP_9, Rml::Input::KI_NUMPAD9 },
			{ GLFW_KEY_KP_ENTER, Rml::Input::KI_NUMPADENTER },
			{ GLFW_KEY_KP_MULTIPLY, Rml::Input::KI_MULTIPLY },      // Asterisk on the numeric keypad.
			{ GLFW_KEY_KP_ADD, Rml::Input::KI_ADD },               // Plus on the numeric keypad.
			//{ GLFW_KEY_KP_SPACE, Rml::Input::KI_SEPARATOR },
			{ GLFW_KEY_KP_SUBTRACT, Rml::Input::KI_SUBTRACT },         // Minus on the numeric keypad.
			{ GLFW_KEY_KP_DECIMAL, Rml::Input::KI_DECIMAL },        // Period on the numeric keypad.
			{ GLFW_KEY_KP_DIVIDE, Rml::Input::KI_DIVIDE },          // Forward Slash on the numeric keypad.
			{ GLFW_KEY_BACKSPACE, Rml::Input::KI_BACK },            // Backspace key.
			{ GLFW_KEY_TAB, Rml::Input::KI_TAB },                   // Tab key.
			//{ GLFW_KEY_CLEAR, Rml::Input::KI_CLEAR },
			{ GLFW_KEY_ENTER, Rml::Input::KI_RETURN },
			{ GLFW_KEY_PAUSE, Rml::Input::KI_PAUSE },
			{ GLFW_KEY_CAPS_LOCK, Rml::Input::KI_CAPITAL },          // Capslock key.
			{ GLFW_KEY_ESCAPE, Rml::Input::KI_ESCAPE },             // Escape key.
			{ GLFW_KEY_PAGE_UP, Rml::Input::KI_PRIOR },              // Page Up key.
			{ GLFW_KEY_PAGE_DOWN, Rml::Input::KI_NEXT },             // Page Down key.
			{ GLFW_KEY_END, Rml::Input::KI_END },
			{ GLFW_KEY_HOME, Rml::Input::KI_HOME },
			{ GLFW_KEY_LEFT, Rml::Input::KI_LEFT },                 // Left Arrow key.
			{ GLFW_KEY_UP, Rml::Input::KI_UP },                     // Up Arrow key.
			{ GLFW_KEY_RIGHT, Rml::Input::KI_RIGHT },               // Right Arrow key.
			{ GLFW_KEY_DOWN, Rml::Input::KI_DOWN },                 // Down Arrow key.
			//{ GLFW_KEY_SELECT, Rml::Input::KI_SELECT },
			{ GLFW_KEY_PRINT_SCREEN, Rml::Input::KI_SNAPSHOT },      // Print Screen key.
			{ GLFW_KEY_INSERT, Rml::Input::KI_INSERT },
			{ GLFW_KEY_DELETE, Rml::Input::KI_DELETE },
			//{ GLFW_KEY_HELP, Rml::Input::KI_HELP },
			{ GLFW_KEY_LEFT_SUPER, Rml::Input::KI_LWIN },                 // Left Windows key.
			{ GLFW_KEY_RIGHT_SUPER, Rml::Input::KI_RWIN },                 // Right Windows key.
			//{ GLFW_KEY_APPLICATION, Rml::Input::KI_APPS },          // Applications key.
			//{ GLFW_KEY_POWER, Rml::Input::KI_POWER },
			//{ GLFW_KEY_SLEEP, Rml::Input::KI_SLEEP },
			{ GLFW_KEY_F1, Rml::Input::KI_F1 },
			{ GLFW_KEY_F2, Rml::Input::KI_F2 },
			{ GLFW_KEY_F3, Rml::Input::KI_F3 },
			{ GLFW_KEY_F4, Rml::Input::KI_F4 },
			{ GLFW_KEY_F5, Rml::Input::KI_F5 },
			{ GLFW_KEY_F6, Rml::Input::KI_F6 },
			{ GLFW_KEY_F7, Rml::Input::KI_F7 },
			{ GLFW_KEY_F8, Rml::Input::KI_F8 },
			{ GLFW_KEY_F9, Rml::Input::KI_F9 },
			{ GLFW_KEY_F10, Rml::Input::KI_F10 },
			{ GLFW_KEY_F11, Rml::Input::KI_F11 },
			{ GLFW_KEY_F12, Rml::Input::KI_F12 },
			{ GLFW_KEY_F13, Rml::Input::KI_F13 },
			{ GLFW_KEY_F14, Rml::Input::KI_F14 },
			{ GLFW_KEY_F15, Rml::Input::KI_F15 },
			{ GLFW_KEY_F16, Rml::Input::KI_F16 },
			{ GLFW_KEY_F17, Rml::Input::KI_F17 },
			{ GLFW_KEY_F18, Rml::Input::KI_F18 },
			{ GLFW_KEY_F19, Rml::Input::KI_F19 },
			{ GLFW_KEY_F20, Rml::Input::KI_F20 },
			{ GLFW_KEY_F21, Rml::Input::KI_F21 },
			{ GLFW_KEY_F22, Rml::Input::KI_F22 },
			{ GLFW_KEY_F23, Rml::Input::KI_F23 },
			{ GLFW_KEY_F24, Rml::Input::KI_F24 },
			{ GLFW_KEY_NUM_LOCK, Rml::Input::KI_NUMLOCK },      // Numlock key.
			{ GLFW_KEY_SCROLL_LOCK, Rml::Input::KI_SCROLL },         // Scroll Lock key.
			{ GLFW_KEY_LEFT_SHIFT, Rml::Input::KI_LSHIFT },
			{ GLFW_KEY_RIGHT_SHIFT, Rml::Input::KI_RSHIFT },
			{ GLFW_KEY_LEFT_CONTROL, Rml::Input::KI_LCONTROL },
			{ GLFW_KEY_RIGHT_CONTROL, Rml::Input::KI_RCONTROL },
			{ GLFW_KEY_LEFT_ALT, Rml::Input::KI_LMENU },
			{ GLFW_KEY_RIGHT_ALT, Rml::Input::KI_RMENU },
			//{ GLFW_KEY_MUTE, Rml::Input::KI_VOLUME_MUTE },
			//{ GLFW_KEY_VOLUME_DOWN, Rml::Input::KI_VOLUME_DOWN },
			//{ GLFW_KEY_VOLUME_UP, Rml::Input::KI_VOLUME_UP },
	};
#endif
	GLFWWindow::GLFWWindow() : ISystemWindow(), m_WindowHandle(nullptr), m_Focused(false),
							   m_CursorMode(ECursorMode::Normal), m_InputManager(Input::Manager_ptr(new Input::Manager(this))),
							   m_SwapChain(nullptr), m_Vsync(true)
	{

	}

	GLFWWindow::~GLFWWindow()
	{
#ifdef GLAD_INSTALL_DEBUG
		gladUninstallGLDebug();
#endif
	}

	void MessageCallback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
	{
		// Note: disabling flood of notifications through glDebugMessageControl() has no effect,
		// so we have to filter them out here
		if (id == 131185 || // Buffer detailed info: Buffer object <X> (bound to GL_XXXX ... , usage hint is GL_DYNAMIC_DRAW)
			// will use VIDEO memory as the source for buffer object operations.
			id == 131186 ||   // Buffer object <X> (bound to GL_XXXX, usage hint is GL_DYNAMIC_DRAW) is being copied/moved from VIDEO memory to HOST memory.
			id == 131204 // Unused texture
				)
			return;

		if(severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

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

	void GLFWWindow::Initialize(const WindowDefinition& windowDefinition, const std::shared_ptr<ISystemWindow>& parentWindow)
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

		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);


		//glfwWindowHint(GLFW_DEPTH_BITS, 32);
		//glfwWindowHint(GLFW_STENCIL_BITS, 0);

		//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
		//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

		if(!windowDefinition.HasOSWindowBorder) {
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		}

		if(windowDefinition.Maximized) {
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_FALSE);

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
		int glVersion;
		if(!(glVersion = gladLoadGL())) {
			printf("Something went wrong!\n");
			exit(-1);
		}
		// During init, enable debug output
#if OPENGL_ERROR_CHECKING
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback( MessageCallback, nullptr );
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
#ifdef GLAD_INSTALL_DEBUG
		gladInstallGLDebug();
#endif

		glGetString(GL_VERSION);
		AU_LOG_INFO("OpenGL ", GLVersion.major, ".", GLVersion.minor);

		if(!GLAD_GL_EXT_texture_array) {
			AU_LOG_ERROR("GLAD_GL_EXT_texture_array not found !");
		}

		/*if(!GLAD_GL_NV_gpu_shader5) {
			AU_LOG_ERROR("GLAD_GL_NV_gpu_shader5 not found !");
		}*/

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



		glDisable(GL_FRAMEBUFFER_SRGB);
	}

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
}
#endif