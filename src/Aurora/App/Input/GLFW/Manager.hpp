#pragma once
#include "Aurora/Core/Common.hpp"

#include "../IManager.hpp"
#include "../../Window.hpp"

#ifdef AU_DEBUG_INPUT
#   ifndef AU_DEBUG_COUT_INPUT
#       define AU_DEBUG_COUT_INPUT(input, value) AU_DEBUG_COUT(input << ": " << value)
#   endif
#else
#   define AU_DEBUG_COUT_INPUT(input, value)
#endif

namespace Aurora::Input
{
    AU_CLASS(Manager) : public IManager
    {
    	friend class ::Aurora::Window;
    protected:
        explicit Manager(Window* window) : IManager(),
                  m_GlfwWindow(window)
        {
        }

    private:
        /// This pointer is managed by std::shared_ptr
        /// Is always valid as
        /// Do not copy this pointer anywhere
        Window* m_GlfwWindow;

    public:
        static const std::size_t MaxKeyCount = 1024;
        static const std::size_t MaxMouseButtonCount = 256;
        static const std::size_t MaxJoystickCount = 16;
    private: //THINK What happens if I manage to press and release the key in single tick? (see Steam Input >> Button)
        std::vector<char8_t>                           m_TextBuffer   = {};
        std::array<bool, MaxKeyCount>                  m_KeyCodes     = {};
        std::array<bool, MaxKeyCount>                  m_ScanCodes    = {};
        std::array<bool, MaxMouseButtonCount>          m_MouseButtons = {};
        std::array<GLFWgamepadstate, MaxJoystickCount> m_Gamepads     = {};
    public:
        typedef uint8_t JoystickIndex_t;
        [[nodiscard]] std::string GetKeyFromGlfw(int glfwKey, int glfwScanCode) const noexcept;
    public:
        void PrePollEvents()
        {
            m_TextBuffer.clear();
            m_ScrollWheelChange = {0, 0};

            m_CursorChange_Pixels     = {0, 0};
            m_CursorChange_Percentage = {0, 0};
            m_CursorPosition_Prev = m_CursorPosition_Tmp;
        }
    public:
        /// Update value of all known bindings
        /// Call after `PrePollEvents()` but before code reading data from `Binding`
        void Update(double delta);
    public:
        void static LoadGamepadConfig(const char* mappingContent);
        void static LoadGamepadConfig(const std::string& mappingContent) { return LoadGamepadConfig(mappingContent.c_str()); }
        void static LoadGamepadConfig(std::istream& in);
        void static LoadGamepadConfig(const std::filesystem::path& mappingFile);
    public:
        void OnKeyChange(int keyCode, int scanCode, bool down);
        void OnTextInput(const std::u8string& utf8)
        {
            m_TextBuffer.reserve(utf8.size());
            for(const char8_t& c : utf8)
                m_TextBuffer.emplace_back(c);
        }
        void OnMouseMove(const glm::dvec2& newPosition);
        void OnMouseWheel(const glm::dvec2& newPosition)
        {
            m_ScrollWheelChange += newPosition;

            AU_DEBUG_COUT_INPUT("mouse_wheel_side", m_ScrollWheelChange.x);
            AU_DEBUG_COUT_INPUT("mouse_wheel",      m_ScrollWheelChange.y);
        }
        void OnMouseButton(int buttonCode, bool down)
        {
            static_assert(GLFW_MOUSE_BUTTON_LEFT   == 0); // LMB
            static_assert(GLFW_MOUSE_BUTTON_RIGHT  == 1); // RMB
            static_assert(GLFW_MOUSE_BUTTON_MIDDLE == 2); // MMB

            if(buttonCode != GLFW_KEY_UNKNOWN && buttonCode >= 0)
            {
                if(buttonCode >= MaxMouseButtonCount) [[unlikely]]
                {
					AU_LOG_ERROR("Mouse button ", buttonCode, " is too high");
                    return;
                }
                m_MouseButtons[buttonCode] = down;

                AU_DEBUG_COUT_INPUT("mouse_" << buttonCode, (down ? "down" : "up"));
            }
            else
            {
				AU_LOG_ERROR("OnMouseButton with invalid buttonCode, why?");
            }
        }
        void OnJoystickConnectChange(JoystickIndex_t joyIndex, bool connected);
        void OnFocusChange(bool focused)
        {
            for(auto& binding : m_KnownBindings)
                binding->m_ValueCurrent = 0;

            AU_DEBUG_COUT_INPUT("Focus changed", (focused ? "focused" : "not focused"));
        }

    // Input Type Detection
    private:
        double m_TimeFromLastGamepadInput = MaxTimeFromLastGamepadInput;
    public:
        /// How many seconds to wait after gamepad input to switch back to Keyboard+Mouse
        static constexpr double MaxTimeFromLastGamepadInput = 30; // 30s
        static bool IsPictogramControllerConnected();

    // Cursor
    private:
                      glm::dvec2  m_CursorPosition_Tmp{};
                      glm::dvec2  m_CursorPosition_Prev{};
        std::optional<glm::dvec2> m_CursorPosition_Pixels     = {};
        std::optional<glm::dvec2> m_CursorPosition_Percentage = {};
                      glm::dvec2  m_CursorChange_Pixels       = {0, 0};
                      glm::dvec2  m_CursorChange_Percentage   = {0, 0};
    public:
        [[nodiscard]] const std::optional<glm::dvec2>& CursorPosition_Pixels()     const noexcept override { return m_CursorPosition_Pixels;          }
        [[nodiscard]] const               glm::dvec2&  CursorChange_Pixels()       const noexcept override { return m_CursorChange_Pixels;            }
        [[nodiscard]] const std::optional<glm::dvec2>& CursorPosition_Percentage() const noexcept override { return m_CursorPosition_Percentage;      }
        [[nodiscard]] const               glm::dvec2&  CursorChange_Percentage()   const noexcept override { return m_CursorChange_Percentage;        }
        [[nodiscard]]                     bool         IsCursorLocked()            const          override { return m_GlfwWindow->GetCursorMode() != ECursorMode::Normal;   }
                                          void         LockCursor(bool locked)     const          override {        m_GlfwWindow->SetCursorMode(locked ? ECursorMode::Disabled : ECursorMode::Normal); }

		bool ActiveCategory(const std::string& category) override;

    // Scroll Wheel
    private:
        glm::dvec2 m_ScrollWheelChange{};
    public:
        //TODO Public

    // Keys
    public:
        [[nodiscard]] bool          IsPressed        (const Key_t& key) override;
        [[nodiscard]] std::u8string GetKeyDisplayName(const Key_t& key) override;

    // Axis
    public:
        [[nodiscard]] double        GetValue        (const Key_t& name) override;
        [[nodiscard]] std::u8string GetValueName    (const Key_t& name) override;

    public:
		const std::vector<char8_t>& GetTypedChars() override { return m_TextBuffer; }
    };
}