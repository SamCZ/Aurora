#include "Window.hpp"
#include "Input/InputManager.hpp"

namespace Aurora::App
{
    static Map<int, Key> GLFWKeyMap;

    bool FWindow::IS_GLFW_CONTEXT_INITIALIZED = false;

    static void InitGLFWKeyMap()
    {
#define MAP(key, keyType) GLFWKeyMap[key] = keyType;

        MAP(GLFW_KEY_SPACE, Keys::SpaceBar)
        MAP(GLFW_KEY_APOSTROPHE, Keys::Apostrophe)
        MAP(GLFW_KEY_COMMA, Keys::Comma)
        MAP(GLFW_KEY_MINUS, Keys::Subtract)
        MAP(GLFW_KEY_PERIOD, Keys::Period)
        MAP(GLFW_KEY_SLASH, Keys::Slash)

        MAP(GLFW_KEY_SEMICOLON, Keys::Semicolon)
        MAP(GLFW_KEY_EQUAL, Keys::Equals)

        MAP(GLFW_KEY_0, Keys::Zero)
        MAP(GLFW_KEY_1, Keys::One)
        MAP(GLFW_KEY_2, Keys::Two)
        MAP(GLFW_KEY_3, Keys::Three)
        MAP(GLFW_KEY_4, Keys::Four)
        MAP(GLFW_KEY_5, Keys::Five)
        MAP(GLFW_KEY_6, Keys::Six)
        MAP(GLFW_KEY_7, Keys::Seven)
        MAP(GLFW_KEY_8, Keys::Eight)
        MAP(GLFW_KEY_9, Keys::Nine)

        MAP(GLFW_KEY_A, Keys::A)
        MAP(GLFW_KEY_B, Keys::B)
        MAP(GLFW_KEY_C, Keys::C)
        MAP(GLFW_KEY_D, Keys::D)
        MAP(GLFW_KEY_E, Keys::E)
        MAP(GLFW_KEY_F, Keys::F)
        MAP(GLFW_KEY_G, Keys::G)
        MAP(GLFW_KEY_H, Keys::H)
        MAP(GLFW_KEY_I, Keys::I)
        MAP(GLFW_KEY_J, Keys::J)
        MAP(GLFW_KEY_K, Keys::K)
        MAP(GLFW_KEY_L, Keys::L)
        MAP(GLFW_KEY_M, Keys::M)
        MAP(GLFW_KEY_N, Keys::N)
        MAP(GLFW_KEY_O, Keys::O)
        MAP(GLFW_KEY_P, Keys::P)
        MAP(GLFW_KEY_Q, Keys::Q)
        MAP(GLFW_KEY_R, Keys::R)
        MAP(GLFW_KEY_S, Keys::S)
        MAP(GLFW_KEY_T, Keys::T)
        MAP(GLFW_KEY_U, Keys::U)
        MAP(GLFW_KEY_V, Keys::V)
        MAP(GLFW_KEY_W, Keys::W)
        MAP(GLFW_KEY_X, Keys::X)
        MAP(GLFW_KEY_Y, Keys::Y)
        MAP(GLFW_KEY_Z, Keys::Z)

        MAP(GLFW_KEY_LEFT_BRACKET, Keys::LeftBracket)
        MAP(GLFW_KEY_BACKSLASH, Keys::Backslash)
        MAP(GLFW_KEY_RIGHT_BRACKET, Keys::RightBracket)
        MAP(GLFW_KEY_GRAVE_ACCENT, Keys::A_AccentGrave)

        MAP(GLFW_KEY_ESCAPE, Keys::Escape)
        MAP(GLFW_KEY_ENTER, Keys::Enter)
        MAP(GLFW_KEY_TAB, Keys::Tab)
        MAP(GLFW_KEY_BACKSPACE, Keys::BackSpace)
        MAP(GLFW_KEY_INSERT, Keys::Insert)
        MAP(GLFW_KEY_DELETE, Keys::Delete)
        MAP(GLFW_KEY_RIGHT, Keys::Right)
        MAP(GLFW_KEY_LEFT, Keys::Left)
        MAP(GLFW_KEY_DOWN, Keys::Down)
        MAP(GLFW_KEY_UP, Keys::Up)
        MAP(GLFW_KEY_PAGE_UP, Keys::PageUp)
        MAP(GLFW_KEY_PAGE_DOWN, Keys::PageDown)
        MAP(GLFW_KEY_HOME, Keys::Home)
        MAP(GLFW_KEY_END, Keys::End)
        MAP(GLFW_KEY_CAPS_LOCK, Keys::CapsLock)
        MAP(GLFW_KEY_SCROLL_LOCK, Keys::ScrollLock)
        MAP(GLFW_KEY_NUM_LOCK, Keys::NumLock)
        //MAP(GLFW_KEY_PRINT_SCREEN, Keys::Invalid)
        MAP(GLFW_KEY_PAUSE, Keys::Pause)

        MAP(GLFW_KEY_F1, Keys::F1)
        MAP(GLFW_KEY_F2, Keys::F2)
        MAP(GLFW_KEY_F3, Keys::F3)
        MAP(GLFW_KEY_F4, Keys::F4)
        MAP(GLFW_KEY_F5, Keys::F5)
        MAP(GLFW_KEY_F6, Keys::F6)
        MAP(GLFW_KEY_F7, Keys::F7)
        MAP(GLFW_KEY_F8, Keys::F8)
        MAP(GLFW_KEY_F9, Keys::F9)
        MAP(GLFW_KEY_F10, Keys::F10)
        MAP(GLFW_KEY_F11, Keys::F11)
        MAP(GLFW_KEY_F12, Keys::F12)

        MAP(GLFW_KEY_KP_0, Keys::NumPadZero)
        MAP(GLFW_KEY_KP_1, Keys::NumPadOne)
        MAP(GLFW_KEY_KP_2, Keys::NumPadTwo)
        MAP(GLFW_KEY_KP_3, Keys::NumPadThree)
        MAP(GLFW_KEY_KP_4, Keys::NumPadFour)
        MAP(GLFW_KEY_KP_5, Keys::NumPadFive)
        MAP(GLFW_KEY_KP_6, Keys::NumPadSix)
        MAP(GLFW_KEY_KP_7, Keys::NumPadSeven)
        MAP(GLFW_KEY_KP_8, Keys::NumPadEight)
        MAP(GLFW_KEY_KP_9, Keys::NumPadNine)

        MAP(GLFW_KEY_KP_DECIMAL, Keys::Decimal)
        MAP(GLFW_KEY_KP_DIVIDE, Keys::Divide)
        MAP(GLFW_KEY_KP_MULTIPLY, Keys::Multiply)
        MAP(GLFW_KEY_KP_SUBTRACT, Keys::Subtract)
        MAP(GLFW_KEY_KP_ADD, Keys::Add)
        MAP(GLFW_KEY_KP_ENTER, Keys::Enter)
        MAP(GLFW_KEY_KP_EQUAL, Keys::Equals)

        MAP(GLFW_KEY_LEFT_SHIFT, Keys::LeftShift)
        MAP(GLFW_KEY_LEFT_CONTROL, Keys::LeftControl)
        MAP(GLFW_KEY_LEFT_ALT, Keys::LeftAlt)

        MAP(GLFW_KEY_LEFT_SUPER, Keys::LeftAlt)

        MAP(GLFW_KEY_RIGHT_SHIFT, Keys::RightShift)
        MAP(GLFW_KEY_RIGHT_CONTROL, Keys::RightControl)
        MAP(GLFW_KEY_RIGHT_ALT, Keys::RightAlt)
#undef MAP
    }

    FWindow::FWindow()
    : FSizeable(), m_WindowHandle(nullptr), m_Focused(false),
    m_CursorMode(ECursorMode::Normal), m_InputManager(MakeShared<FInputManager>(this)),
    m_SwapChain(nullptr), m_Vsync(true)
    {

    }

    FWindow::~FWindow() = default;

    void FWindow::Initialize(const App::FWindowDefinition& windowDefinition, const SharedPtr<App::FWindow>& parentWindow)
    {
        if(!IS_GLFW_CONTEXT_INITIALIZED)
        {
            glfwInit();
            Keys::Initialize();
            InitGLFWKeyMap();
            IS_GLFW_CONTEXT_INITIALIZED = true;
        }

        m_Title = windowDefinition.Title;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        if(!windowDefinition.HasOSWindowBorder) {
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        }

        if(windowDefinition.Maximized) {
            glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        }

        SetSize({windowDefinition.Width, windowDefinition.Height});

        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        const GLFWvidmode* vidMode = glfwGetVideoMode(primary);

        m_WindowHandle = glfwCreateWindow(GetSize().x, GetSize().y, windowDefinition.Title.c_str(), nullptr, nullptr);

        if(!m_WindowHandle) {
            std::cerr << "Cannot create window !" << std::endl;
            glfwTerminate();
            exit(1);
            return;
        }

        if(parentWindow != nullptr && !windowDefinition.Maximized) {
            // TODO: Center relative to parent
        } else if(!windowDefinition.Maximized) {
            int x = vidMode->width / 2 - windowDefinition.Width / 2;
            int y = vidMode->height / 2 - windowDefinition.Height / 2;
            glfwSetWindowPos(m_WindowHandle, x, y);
        }

        if(parentWindow != nullptr) {
#if PLATFORM_WINDOWS
            HWND hwNative = glfwGetWin32Window(WindowHandle);
            HWND parentHwNative = glfwGetWin32Window(InParent->GetWindowHandle());

            ::SetParent(hwNative, parentHwNative);
#endif
        }

        glfwSetInputMode(m_WindowHandle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

        glfwSetWindowUserPointer(m_WindowHandle, this);

        glfwSetWindowSizeCallback(m_WindowHandle, FWindow::OnResizeCallback);
        glfwSetWindowFocusCallback(m_WindowHandle, FWindow::OnFocusCallback);

        glfwSetKeyCallback(m_WindowHandle, FWindow::OnKeyCallback);
        glfwSetCursorPosCallback(m_WindowHandle, FWindow::OnCursorPosCallBack);
        glfwSetScrollCallback(m_WindowHandle, FWindow::OnMouseScrollCallback);
        glfwSetMouseButtonCallback(m_WindowHandle, FWindow::OnMouseButtonCallback);
    }

    void FWindow::Show()
    {
        if(m_WindowHandle != nullptr)
            glfwShowWindow(m_WindowHandle);
    }

    void FWindow::Hide()
    {
        if(m_WindowHandle != nullptr)
            glfwHideWindow(m_WindowHandle);
    }

    void FWindow::Destroy()
    {
        if(m_WindowHandle != nullptr)
            glfwDestroyWindow(m_WindowHandle);
    }

    void FWindow::Minimize()
    {
        if(m_WindowHandle != nullptr)
            glfwIconifyWindow(m_WindowHandle);
    }

    void FWindow::Maximize()
    {
        if(m_WindowHandle != nullptr)
            glfwMaximizeWindow(m_WindowHandle);
    }

    void FWindow::Restore()
    {
        if(m_WindowHandle != nullptr)
            glfwRestoreWindow(m_WindowHandle);
    }

    void FWindow::Focus()
    {
        if(m_WindowHandle != nullptr)
            glfwFocusWindow(m_WindowHandle);
    }

    void FWindow::SetTitle(const String &title)
    {
        glfwSetWindowTitle(m_WindowHandle, title.c_str());
    }

    GLFWwindow* FWindow::GetWindowHandle()
    {
        return m_WindowHandle;
    }

    bool FWindow::IsShouldClose() const
    {
        if(m_WindowHandle == nullptr)
            return false;

        return glfwWindowShouldClose(m_WindowHandle);
    }

    void FWindow::SetCursorMode(const ECursorMode& mode)
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

    const ECursorMode &FWindow::GetCursorMode() const
    {
        return m_CursorMode;
    }

    bool FWindow::IsFocused() const
    {
        return m_Focused;
    }

    bool FWindow::IsIconified()
    {
        return glfwGetWindowAttrib(m_WindowHandle, GLFW_ICONIFIED) == GLFW_TRUE;
    }

    FInputManagerPtr FWindow::GetInputManager()
    {
        return m_InputManager;
    }

    void FWindow::OnResizeCallback(GLFWwindow* rawWindow, int width, int height)
    {
        auto* window = static_cast<FWindow*>(glfwGetWindowUserPointer(rawWindow));
        window->SetSize({width, height});
    }

    void FWindow::OnFocusCallback(GLFWwindow *rawWindow, int focused)
    {
        auto* window = static_cast<FWindow*>(glfwGetWindowUserPointer(rawWindow));
        window->m_Focused = focused;

        //TODO: Call callbacks
    }

    void FWindow::OnKeyCallback(GLFWwindow *rawWindow, int key, int scancode, int action, int mods)
    {
        Key hkey;

        if(GLFWKeyMap.contains(key)) {
            hkey = GLFWKeyMap[key];
        } else {
            hkey = Keys::Invalid;
        }

#ifdef DEBUG
        const char* keyName = glfwGetKeyName(key, scancode);

		if(hkey == Keys::Invalid) {
			Log("Pressed invalid key (" + ToString(key) + ") !");
			return;
		}
#endif

        InputEvent event;

        switch (action)
        {
            case GLFW_PRESS:
                event = IE_Pressed;
                break;
            case GLFW_RELEASE:
                event = IE_Released;
                break;
            case GLFW_REPEAT:
                event = IE_Repeat;
                break;
            default:
                return;
        }

        auto* window = static_cast<FWindow*>(glfwGetWindowUserPointer(rawWindow));
        window->m_InputManager->OnKeyAction(hkey, event);
    }

    void FWindow::OnCursorPosCallBack(GLFWwindow *rawWindow, double xpos, double ypos)
    {
        auto* window = static_cast<FWindow*>(glfwGetWindowUserPointer(rawWindow));

        window->m_InputManager->UpdateMousePos(static_cast<int>(xpos), static_cast<int>(ypos));
    }

    void FWindow::OnMouseScrollCallback(GLFWwindow *rawWindow, double xoffset, double yoffset)
    {
        auto* window = static_cast<FWindow*>(glfwGetWindowUserPointer(rawWindow));
        window->m_InputManager->OnMouseScrollAction(static_cast<float>(xoffset), static_cast<double>(yoffset));
    }

    void FWindow::OnMouseButtonCallback(GLFWwindow *rawWindow, int button, int action, int mods)
    {
        InputEvent event;

        switch (action)
        {
            case GLFW_PRESS:
                event = IE_Pressed;
                break;
            case GLFW_RELEASE:
                event = IE_Released;
                break;
            case GLFW_REPEAT:
                event = IE_Repeat;
                break;
            default:
                return;
        }

        EMouseButtons buttonType;

        switch (button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
                buttonType = EMouseButtons::Left;
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                buttonType = EMouseButtons::Middle;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                buttonType = EMouseButtons::Right;
                break;
            case GLFW_MOUSE_BUTTON_4:
                buttonType = EMouseButtons::Thumb01;
                break;
            case GLFW_MOUSE_BUTTON_5:
                buttonType = EMouseButtons::Thumb02;
                break;
            default:
                return;
        }

        auto* window = static_cast<FWindow*>(glfwGetWindowUserPointer(rawWindow));
        window->m_InputManager->OnMouseAction(buttonType, event);
    }
}