#pragma once

#if GLFW_ENABLED

#include "Aurora/Core/Common.hpp"

#include "../IManager.hpp"
#include "../../GLFWWindow.hpp"

#ifdef AU_DEBUG_INPUT
#   ifndef AU_DEBUG_COUT_INPUT
#       define AU_DEBUG_COUT_INPUT(input, value) std::cout << input << ": " << value << std::endl
#   endif
#else
#   ifndef AU_DEBUG_COUT_INPUT
#      define AU_DEBUG_COUT_INPUT(input, value) do {} while(false)
#   endif
#endif

namespace Aurora::Input
{
    AU_CLASS(Manager) : public IManager
    {
    	friend class ::Aurora::GLFWWindow;
    protected:
        explicit Manager(GLFWWindow* window) : IManager(), m_GlfwWindow(window)
        {
        }

    private:
        /// This pointer is managed by std::shared_ptr
        /// Is always valid as
        /// Do not copy this pointer anywhere
		GLFWWindow* m_GlfwWindow;

    public:
        static const std::size_t MaxKeyCount         = 400;
        static const std::size_t MaxKeyScanCount     = 1; // This effectively disables scancodes = only known keys
        static const std::size_t MaxMouseButtonCount = 16;
    private: //THINK What happens if I manage to press and release the key in single tick? (see Steam Input >> Button)
        std::vector<char8_t>                  m_TextBuffer   = {};
        struct KeyStates
        {
            std::array<bool, MaxKeyCount>         KeyCodes     = {};
            std::array<bool, MaxKeyScanCount>     ScanCodes    = {};
            std::array<bool, MaxMouseButtonCount> MouseButtons = {};

            void Clear()
            {
                std::fill(KeyCodes.begin(),     KeyCodes.end(),     false);
                std::fill(ScanCodes.begin(),    ScanCodes.end(),    false);
                std::fill(MouseButtons.begin(), MouseButtons.end(), false);
            }
        };
        KeyStates m_CurrentKeys[2];
        int m_CurrentKeysIndex = 0;
        inline KeyStates& CurrentKeys() { return m_CurrentKeys[m_CurrentKeysIndex]; }
    public:
        [[nodiscard]] std::string GetKeyFromGlfw(int glfwKey, int glfwScanCode) const noexcept;
    private:
        void Clear()
        {
            m_TextBuffer.clear();
            m_ScrollWheelChange = {0, 0};

            m_CursorChange = {0, 0};
            m_CursorPosition_Prev = m_CursorPosition_Tmp;

            CurrentKeys().Clear();
        }
    public:
        void PrePollEvents()
        {
            m_TextBuffer.clear();
            m_ScrollWheelChange = {0, 0};

            m_CursorChange = {0, 0};
            m_CursorPosition_Prev = m_CursorPosition_Tmp;

            m_CurrentKeysIndex = 1 - m_CurrentKeysIndex; // Toggles between 0 and 1
            m_CurrentKeys[m_CurrentKeysIndex] = m_CurrentKeys[1 - m_CurrentKeysIndex];
        }
        /// `Manager::PrePollEvents()` -> `glfwPollEvents()` -> `Manager::Update(...)`
        void Update(double delta)
        {
        }
    public: // GLFW Events
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
        void OnMouseButton(int buttonCode, bool down);
        void OnFocusChange(bool focused)
        {
            Clear();

            AU_DEBUG_COUT_INPUT("Focus changed", (focused ? "focused" : "not focused"));
        }

    // Cursor
    private:
        /// Helper variable for processing position of locked cursor, to work even when switching between locked and unlocked mode.
                      glm::dvec2  m_CursorPosition_Tmp   = {}; //TODO Is really needed?
        /// Position of the cursor during previous tick
                      glm::dvec2  m_CursorPosition_Prev  = {};
        std::optional<glm::dvec2> m_CursorPosition       = {};
        std::optional<glm::dvec2> m_CursorPositionChange = {};
        std::optional<glm::dvec2> m_CursorChange         = {};
    public:
        [[nodiscard]] const std::optional<glm::dvec2>& CursorPosition()       const noexcept override { return m_CursorPosition; }
        [[nodiscard]]                     glm::dvec2   CursorPositionChange() const noexcept override { return m_CursorPositionChange.has_value() ? m_CursorPositionChange.value() : glm::dvec2{0, 0}; }
        [[nodiscard]]                     glm::dvec2   CursorChange()         const noexcept override { return m_CursorChange.has_value()         ? m_CursorChange.value()         : glm::dvec2{0, 0}; }
    public:
        [[nodiscard]] bool IsCursorLocked()        const override { return m_GlfwWindow->GetCursorMode() != ECursorMode::Normal;   }
                      void LockCursor(bool locked) const override {        m_GlfwWindow->SetCursorMode(locked ? ECursorMode::Disabled : ECursorMode::Normal); }

		bool ActiveCategory(const std::string& category) override;

    // Scroll Wheel
    private:
        glm::dvec2 m_ScrollWheelChange{};
    public:
        glm::dvec2 Scrolling() override { return m_ScrollWheelChange; }

    public:
		const std::vector<char8_t>& GetTypedChars() override { return m_TextBuffer; }

    private:
        double GetValue(const IManager::Key_t& name, int keysIndex);
    public:
        /// Get analog value between -1.0 and 1.0
        double GetValue     (const IManager::Key_t& name) { return GetValue(name,     m_CurrentKeysIndex); }
        double GetValue_Prev(const IManager::Key_t& name) { return GetValue(name, 1 - m_CurrentKeysIndex); }
        [[deprecated("Using `GetValue(...) > 0` should work better")]]
        bool IsPressed(const IManager::Key_t& key);

    public:
        double GetAnalog     (const Action_t& action) override;
        double GetAnalog_Prev(const Action_t& action);
    public:
        bool   GetDigital         (const Action_t& action) override { return  GetAnalog(action) > 0; }
        bool   GetDigital_Pressed (const Action_t& action) override { return (GetAnalog(action) > 0) > (GetAnalog_Prev(action) > 0); }
        bool   GetDigital_Released(const Action_t& action) override { return (GetAnalog(action) > 0) < (GetAnalog_Prev(action) > 0); }
    };
}
#endif