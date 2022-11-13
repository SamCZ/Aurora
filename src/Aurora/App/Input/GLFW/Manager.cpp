#if GLFW_ENABLED

#include "Manager.hpp"

#include <iostream>

namespace Aurora::Input
{
    void Manager::OnKeyChange(int keyCode, int scanCode, bool down)
    {
        if(keyCode != GLFW_KEY_UNKNOWN && keyCode >= 0)
        {
            if(keyCode >= MaxKeyCount) [[unlikely]]
            {
				AU_LOG_ERROR("Keycode ", keyCode, " is too high");
                return;
            }
            CurrentKeys().KeyCodes[keyCode] = down;
            AU_DEBUG_COUT_INPUT("keyboard_code_" << keyCode, (down ? "down" : "up"));
        }
        else if(scanCode != GLFW_KEY_UNKNOWN && scanCode >= 0)
        {
            if(scanCode >= MaxKeyScanCount) [[unlikely]]
            {
				AU_LOG_ERROR("Keycode ", keyCode, " is too high");
                return;
            }
            CurrentKeys().ScanCodes[scanCode] = down;
            AU_DEBUG_COUT_INPUT("keyboard_scancode_" << keyCode, (down ? "down" : "up"));
        }
        else
        {
			AU_LOG_ERROR("OnKeyChange with no keyCode and scanCode, why?");
            return;
        }
    }

    void Manager::OnMouseMove(const glm::dvec2& newPosition)
    {
        bool cursorLocked = IsCursorLocked();

        if(cursorLocked)
        {
            // Hidden
            glm::dvec2 change = newPosition - m_CursorPosition_Prev;
            m_CursorChange         = change * CursorSensitivity();
            m_CursorPositionChange = change;

            // Visible
            m_CursorPosition = {};
        }
        else // !cursorLocked
        {
            // Hidden
            m_CursorChange         = {};
            m_CursorPositionChange = {};

            // Visible
            m_CursorPosition = newPosition;
        }
        m_CursorPosition_Tmp = newPosition;

        AU_DEBUG_COUT_INPUT("mouse_x", m_CursorChange_Pixels.x);
        AU_DEBUG_COUT_INPUT("mouse_y", m_CursorChange_Pixels.y);
    }

    void Manager::OnMouseButton(int buttonCode, bool down)
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
            CurrentKeys().MouseButtons[buttonCode] = down;

            AU_DEBUG_COUT_INPUT("mouse_" << buttonCode, (down ? "down" : "up"));
        }
        else
        {
            AU_LOG_ERROR("OnMouseButton with invalid buttonCode, why?");
        }
    }

#pragma region GLFW Key Map
    std::string Manager::GetKeyFromGlfw(int glfwKey, int glfwScanCode) const noexcept
    {
        switch(glfwKey)
        {
#define AU_KEY(glfw, name) case glfw: return name;
            AU_KEY(GLFW_KEY_A, "a")
            AU_KEY(GLFW_KEY_B, "b")
            AU_KEY(GLFW_KEY_C, "c")
            AU_KEY(GLFW_KEY_D, "d")
            AU_KEY(GLFW_KEY_E, "e")
            AU_KEY(GLFW_KEY_F, "f")
            AU_KEY(GLFW_KEY_G, "g")
            AU_KEY(GLFW_KEY_H, "h")
            AU_KEY(GLFW_KEY_I, "i")
            AU_KEY(GLFW_KEY_J, "j")
            AU_KEY(GLFW_KEY_K, "k")
            AU_KEY(GLFW_KEY_L, "l")
            AU_KEY(GLFW_KEY_M, "m")
            AU_KEY(GLFW_KEY_N, "n")
            AU_KEY(GLFW_KEY_O, "o")
            AU_KEY(GLFW_KEY_P, "p")
            AU_KEY(GLFW_KEY_Q, "q")
            AU_KEY(GLFW_KEY_R, "r")
            AU_KEY(GLFW_KEY_S, "s")
            AU_KEY(GLFW_KEY_T, "t")
            AU_KEY(GLFW_KEY_U, "u")
            AU_KEY(GLFW_KEY_V, "v")
            AU_KEY(GLFW_KEY_W, "w")
            AU_KEY(GLFW_KEY_X, "x")
            AU_KEY(GLFW_KEY_Y, "y")
            AU_KEY(GLFW_KEY_Z, "z")

            AU_KEY(GLFW_KEY_0, "0")
            AU_KEY(GLFW_KEY_1, "1")
            AU_KEY(GLFW_KEY_2, "2")
            AU_KEY(GLFW_KEY_3, "3")
            AU_KEY(GLFW_KEY_4, "4")
            AU_KEY(GLFW_KEY_5, "5")
            AU_KEY(GLFW_KEY_6, "6")
            AU_KEY(GLFW_KEY_7, "7")
            AU_KEY(GLFW_KEY_8, "8")
            AU_KEY(GLFW_KEY_9, "9")

            AU_KEY(GLFW_KEY_APOSTROPHE,    "apostrophe"   ) /* ' */
            AU_KEY(GLFW_KEY_COMMA,         "comma"        ) /* , */
            AU_KEY(GLFW_KEY_MINUS,         "minus"        ) /* - */
            AU_KEY(GLFW_KEY_PERIOD,        "period"       ) /* . */
            AU_KEY(GLFW_KEY_SLASH,         "slash"        ) /* / */
            AU_KEY(GLFW_KEY_LEFT_BRACKET,  "bracket_left" ) /* [ */
            AU_KEY(GLFW_KEY_BACKSLASH,     "backslash"    ) /* \ */
            AU_KEY(GLFW_KEY_RIGHT_BRACKET, "bracket_right") /* ] */
            AU_KEY(GLFW_KEY_GRAVE_ACCENT,  "grave_accent" ) /* ` */

            AU_KEY(GLFW_KEY_LEFT_CONTROL,  "ctrl_left"  )
            AU_KEY(GLFW_KEY_RIGHT_CONTROL, "ctrl_right" )

            AU_KEY(GLFW_KEY_LEFT_SHIFT,    "shift_left" )
            AU_KEY(GLFW_KEY_RIGHT_SHIFT,   "shift_right")

            AU_KEY(GLFW_KEY_LEFT_ALT,      "alt_left"   )
            AU_KEY(GLFW_KEY_RIGHT_ALT,     "alt_right"  )

            AU_KEY(GLFW_KEY_LEFT_SUPER,    "super_left" )
            AU_KEY(GLFW_KEY_RIGHT_SUPER,   "super_right")

            AU_KEY(GLFW_KEY_MENU,          "menu"       )

            AU_KEY(GLFW_KEY_ESCAPE,    "escape"   )
            AU_KEY(GLFW_KEY_SPACE,     "spacebar" )
            AU_KEY(GLFW_KEY_TAB,       "tab"      )
            AU_KEY(GLFW_KEY_CAPS_LOCK, "caps_lock")
            AU_KEY(GLFW_KEY_ENTER,     "enter"    )
            AU_KEY(GLFW_KEY_BACKSPACE, "backspace")

            AU_KEY(GLFW_KEY_INSERT,    "insert")
            AU_KEY(GLFW_KEY_DELETE,    "delete")
            AU_KEY(GLFW_KEY_HOME,      "home")
            AU_KEY(GLFW_KEY_END,       "end")
            AU_KEY(GLFW_KEY_PAGE_UP,   "page_up")
            AU_KEY(GLFW_KEY_PAGE_DOWN, "page_down")

            AU_KEY(GLFW_KEY_PRINT_SCREEN, "print_screen")
            AU_KEY(GLFW_KEY_SCROLL_LOCK,  "scroll_lock" )
            AU_KEY(GLFW_KEY_PAUSE,        "pause"       )

            AU_KEY(GLFW_KEY_UP,    "arrow_up"   )
            AU_KEY(GLFW_KEY_DOWN,  "arrow_down" )
            AU_KEY(GLFW_KEY_LEFT,  "arrow_left" )
            AU_KEY(GLFW_KEY_RIGHT, "arrow_right")

            AU_KEY(GLFW_KEY_F1,  "f1" )
            AU_KEY(GLFW_KEY_F2,  "f2" )
            AU_KEY(GLFW_KEY_F3,  "f3" )
            AU_KEY(GLFW_KEY_F4,  "f4" )
            AU_KEY(GLFW_KEY_F5,  "f5" )
            AU_KEY(GLFW_KEY_F6,  "f6" )
            AU_KEY(GLFW_KEY_F7,  "f7" )
            AU_KEY(GLFW_KEY_F8,  "f8" )
            AU_KEY(GLFW_KEY_F9,  "f9" )
            AU_KEY(GLFW_KEY_F10, "f10")
            AU_KEY(GLFW_KEY_F11, "f11")
            AU_KEY(GLFW_KEY_F12, "f12")

            AU_KEY(GLFW_KEY_NUM_LOCK,    "num_lock"    )
            AU_KEY(GLFW_KEY_KP_0,        "num_0"       )
            AU_KEY(GLFW_KEY_KP_1,        "num_1"       )
            AU_KEY(GLFW_KEY_KP_2,        "num_2"       )
            AU_KEY(GLFW_KEY_KP_3,        "num_3"       )
            AU_KEY(GLFW_KEY_KP_4,        "num_4"       )
            AU_KEY(GLFW_KEY_KP_5,        "num_5"       )
            AU_KEY(GLFW_KEY_KP_6,        "num_6"       )
            AU_KEY(GLFW_KEY_KP_7,        "num_7"       )
            AU_KEY(GLFW_KEY_KP_8,        "num_8"       )
            AU_KEY(GLFW_KEY_KP_9,        "num_9"       )
            AU_KEY(GLFW_KEY_KP_DECIMAL,  "num_decimal" )
            AU_KEY(GLFW_KEY_KP_MULTIPLY, "num_multiply")
            AU_KEY(GLFW_KEY_KP_DIVIDE,   "num_divide"  )
            AU_KEY(GLFW_KEY_KP_ADD,      "num_add"     )
            AU_KEY(GLFW_KEY_KP_SUBTRACT, "num_subtract")
            AU_KEY(GLFW_KEY_KP_ENTER,    "num_enter"   )
#undef AU_KEY
            default:
                if(glfwKey != GLFW_KEY_UNKNOWN)
                    return "keyboard_code_" + std::to_string(glfwKey);
                if(glfwScanCode != GLFW_KEY_UNKNOWN)
                    return "keyboard_scancode_" + std::to_string(glfwScanCode);
                return std::string();
        }
    }

    static const std::map<std::string, int> keyNameToGlfw = { // NOLINT(cert-err58-cpp)
#define AU_KEY(glfw, name) { name, glfw }
            AU_KEY( GLFW_KEY_A, "a" ),
            AU_KEY( GLFW_KEY_B, "b" ),
            AU_KEY( GLFW_KEY_C, "c" ),
            AU_KEY( GLFW_KEY_D, "d" ),
            AU_KEY( GLFW_KEY_E, "e" ),
            AU_KEY( GLFW_KEY_F, "f" ),
            AU_KEY( GLFW_KEY_G, "g" ),
            AU_KEY( GLFW_KEY_H, "h" ),
            AU_KEY( GLFW_KEY_I, "i" ),
            AU_KEY( GLFW_KEY_J, "j" ),
            AU_KEY( GLFW_KEY_K, "k" ),
            AU_KEY( GLFW_KEY_L, "l" ),
            AU_KEY( GLFW_KEY_M, "m" ),
            AU_KEY( GLFW_KEY_N, "n" ),
            AU_KEY( GLFW_KEY_O, "o" ),
            AU_KEY( GLFW_KEY_P, "p" ),
            AU_KEY( GLFW_KEY_Q, "q" ),
            AU_KEY( GLFW_KEY_R, "r" ),
            AU_KEY( GLFW_KEY_S, "s" ),
            AU_KEY( GLFW_KEY_T, "t" ),
            AU_KEY( GLFW_KEY_U, "u" ),
            AU_KEY( GLFW_KEY_V, "v" ),
            AU_KEY( GLFW_KEY_W, "w" ),
            AU_KEY( GLFW_KEY_X, "x" ),
            AU_KEY( GLFW_KEY_Y, "y" ),
            AU_KEY( GLFW_KEY_Z, "z" ),

            AU_KEY( GLFW_KEY_0, "0" ),
            AU_KEY( GLFW_KEY_1, "1" ),
            AU_KEY( GLFW_KEY_2, "2" ),
            AU_KEY( GLFW_KEY_3, "3" ),
            AU_KEY( GLFW_KEY_4, "4" ),
            AU_KEY( GLFW_KEY_5, "5" ),
            AU_KEY( GLFW_KEY_6, "6" ),
            AU_KEY( GLFW_KEY_7, "7" ),
            AU_KEY( GLFW_KEY_8, "8" ),
            AU_KEY( GLFW_KEY_9, "9" ),

            AU_KEY( GLFW_KEY_APOSTROPHE,    "apostrophe"    ), /* ' */
            AU_KEY( GLFW_KEY_COMMA,         "comma"         ), /* , */
            AU_KEY( GLFW_KEY_MINUS,         "minus"         ), /* - */
            AU_KEY( GLFW_KEY_PERIOD,        "period"        ), /* . */
            AU_KEY( GLFW_KEY_SLASH,         "slash"         ), /* / */
            AU_KEY( GLFW_KEY_LEFT_BRACKET,  "bracket_left"  ), /* [ */
            AU_KEY( GLFW_KEY_BACKSLASH,     "backslash"     ), /* \ */
            AU_KEY( GLFW_KEY_RIGHT_BRACKET, "bracket_right" ), /* ] */
            AU_KEY( GLFW_KEY_GRAVE_ACCENT,  "grave_accent"  ), /* ` */

            AU_KEY( GLFW_KEY_LEFT_CONTROL,  "ctrl_left"  ),
            AU_KEY( GLFW_KEY_RIGHT_CONTROL, "ctrl_right" ),

            AU_KEY( GLFW_KEY_LEFT_SHIFT,  "shift_left"  ),
            AU_KEY( GLFW_KEY_RIGHT_SHIFT, "shift_right" ),

            AU_KEY( GLFW_KEY_LEFT_ALT,  "alt_left"  ),
            AU_KEY( GLFW_KEY_RIGHT_ALT, "alt_right" ),

            AU_KEY( GLFW_KEY_LEFT_SUPER,  "super_left"  ),
            AU_KEY( GLFW_KEY_RIGHT_SUPER, "super_right" ),

            AU_KEY( GLFW_KEY_MENU, "menu" ),

            AU_KEY( GLFW_KEY_ESCAPE,    "escape"    ),
            AU_KEY( GLFW_KEY_SPACE,     "spacebar"  ),
            AU_KEY( GLFW_KEY_TAB,       "tab"       ),
            AU_KEY( GLFW_KEY_CAPS_LOCK, "caps_lock" ),
            AU_KEY( GLFW_KEY_ENTER,     "enter"     ),
            AU_KEY( GLFW_KEY_BACKSPACE, "backspace" ),

            AU_KEY( GLFW_KEY_INSERT,    "insert"    ),
            AU_KEY( GLFW_KEY_DELETE,    "delete"    ),
            AU_KEY( GLFW_KEY_HOME,      "home"      ),
            AU_KEY( GLFW_KEY_END,       "end"       ),
            AU_KEY( GLFW_KEY_PAGE_UP,   "page_up"   ),
            AU_KEY( GLFW_KEY_PAGE_DOWN, "page_down" ),

            AU_KEY( GLFW_KEY_PRINT_SCREEN, "print_screen" ),
            AU_KEY( GLFW_KEY_SCROLL_LOCK,  "scroll_lock"  ),
            AU_KEY( GLFW_KEY_PAUSE,        "pause"        ),

            AU_KEY( GLFW_KEY_UP,    "arrow_up" ),
            AU_KEY( GLFW_KEY_DOWN,  "arrow_down" ),
            AU_KEY( GLFW_KEY_LEFT,  "arrow_left" ),
            AU_KEY( GLFW_KEY_RIGHT, "arrow_right" ),

            AU_KEY( GLFW_KEY_F1,  "f1"  ),
            AU_KEY( GLFW_KEY_F2,  "f2"  ),
            AU_KEY( GLFW_KEY_F3,  "f3"  ),
            AU_KEY( GLFW_KEY_F4,  "f4"  ),
            AU_KEY( GLFW_KEY_F5,  "f5"  ),
            AU_KEY( GLFW_KEY_F6,  "f6"  ),
            AU_KEY( GLFW_KEY_F7,  "f7"  ),
            AU_KEY( GLFW_KEY_F8,  "f8"  ),
            AU_KEY( GLFW_KEY_F9,  "f9"  ),
            AU_KEY( GLFW_KEY_F10, "f10" ),
            AU_KEY( GLFW_KEY_F11, "f11" ),
            AU_KEY( GLFW_KEY_F12, "f12" ),

            AU_KEY( GLFW_KEY_NUM_LOCK,    "num_lock"     ),
            AU_KEY( GLFW_KEY_KP_0,        "num_0"        ),
            AU_KEY( GLFW_KEY_KP_1,        "num_1"        ),
            AU_KEY( GLFW_KEY_KP_2,        "num_2"        ),
            AU_KEY( GLFW_KEY_KP_3,        "num_3"        ),
            AU_KEY( GLFW_KEY_KP_4,        "num_4"        ),
            AU_KEY( GLFW_KEY_KP_5,        "num_5"        ),
            AU_KEY( GLFW_KEY_KP_6,        "num_6"        ),
            AU_KEY( GLFW_KEY_KP_7,        "num_7"        ),
            AU_KEY( GLFW_KEY_KP_8,        "num_8"        ),
            AU_KEY( GLFW_KEY_KP_9,        "num_9"        ),
            AU_KEY( GLFW_KEY_KP_DECIMAL,  "num_decimal"  ),
            AU_KEY( GLFW_KEY_KP_MULTIPLY, "num_multiply" ),
            AU_KEY( GLFW_KEY_KP_DIVIDE,   "num_divide"   ),
            AU_KEY( GLFW_KEY_KP_ADD,      "num_add"      ),
            AU_KEY( GLFW_KEY_KP_SUBTRACT, "num_subtract" ),
            AU_KEY( GLFW_KEY_KP_ENTER,    "num_enter"    ),
#undef AU_KEY

            { "ctrl",  GLFW_KEY_LEFT_CONTROL },
            { "shift", GLFW_KEY_LEFT_SHIFT   },
            { "alt",   GLFW_KEY_LEFT_ALT     },
            { "super", GLFW_KEY_LEFT_SUPER   }
    };
#pragma endregion

    [[nodiscard]] inline bool GetGlfwKey_Keyboard(const std::string& key, int* keyCode, int* scanCode)
    {
        auto it = keyNameToGlfw.find(key);
        if(it != keyNameToGlfw.end())
        {
            if(scanCode != nullptr)
                *scanCode = glfwGetKeyScancode(it->second);
            if(keyCode != nullptr)
                *keyCode = it->second;
            return true;
        }

        if(key.starts_with("keyboard_code_"))
        {
            if(scanCode != nullptr)
                *scanCode = GLFW_KEY_UNKNOWN;
            if(keyCode != nullptr)
                *keyCode = std::strtol(key.data() + 14, nullptr, 10);
            return true;
        }
        if(key.starts_with("keyboard_scancode_"))
        {
            if(scanCode != nullptr)
                *scanCode = std::strtol(key.data() + 18, nullptr, 10);
            if(keyCode != nullptr)
                *keyCode = GLFW_KEY_UNKNOWN;
            return true;
        }

        return false; // Not a keyboard key
    }

    bool Manager::IsPressed(const IManager::Key_t& key)
    {
        // Keyboard
        {
            int keyCode, scanCode;
            if(GetGlfwKey_Keyboard(key, &keyCode, &scanCode))
            {
                if(keyCode != GLFW_KEY_UNKNOWN)
                {
                    if(keyCode < CurrentKeys().KeyCodes.size())
                        return CurrentKeys().KeyCodes[keyCode];
                    else
                        return false;
                }

                if(scanCode != GLFW_KEY_UNKNOWN)
                {
                    if(scanCode < CurrentKeys().ScanCodes.size())
                        return CurrentKeys().ScanCodes[scanCode];
                    else
                        return false;
                }

				AU_LOG_WARNING("Unknown `IsPressed` key requested: ", key);
                return false;
            }
        }

        // Mouse
        if(key.starts_with("mouse_"))
        {
            try
            {
                int index = std::stoi(key.data() + 6);
                if(index < CurrentKeys().MouseButtons.size())
                    return CurrentKeys().MouseButtons[index];
                else
                    return false;
            }
            catch(...)
            {
            }

			AU_LOG_WARNING("Unknown `IsPressed` mouse button requested: ", key);
            return false;
        }

        return false;
    }

    double Manager::GetValue(const IManager::Key_t& name, int keysIndex)
    {
        // Keyboard
        {
            int keyCode, scanCode;
            if(GetGlfwKey_Keyboard(name, &keyCode, &scanCode))
            {
                if(keyCode != GLFW_KEY_UNKNOWN)
                {
                    if(keyCode < m_CurrentKeys[keysIndex].KeyCodes.size())
                        return m_CurrentKeys[keysIndex].KeyCodes[keyCode] ? 1.0 : 0.0;
                    else
                        return 0.0;
                }

                if(scanCode != GLFW_KEY_UNKNOWN)
                {
                    if(scanCode < m_CurrentKeys[keysIndex].ScanCodes.size())
                        return m_CurrentKeys[keysIndex].ScanCodes[scanCode] ? 1.0 : 0.0;
                    else
                        return 0.0;
                }

				AU_LOG_INFO("Unknown `GetValue` key requested: ", name);
                return 0.0;
            }
        }

        // Mouse
        if(name.starts_with("mouse_"))
        {
            if(name == "mouse_x")
                return m_CursorChange.has_value() ? m_CursorChange.value().x : 0.0;
            if(name == "mouse_y")
                return m_CursorChange.has_value() ? m_CursorChange.value().y : 0.0;

            if(name == "mouse_wheel")
                return m_ScrollWheelChange.y;
            if(name == "mouse_wheel_side")
                return m_ScrollWheelChange.x;

            try
            {
                int index = std::stoi(name.data() + 6);
                if(index < m_CurrentKeys[keysIndex].MouseButtons.size())
                    return m_CurrentKeys[keysIndex].MouseButtons[index] ? 1.0 : 0.0;
                else
                    return 0.0;
            }
            catch(...)
            {
            }

			AU_LOG_INFO("Unknown `GetValue` mouse button/axis requested: ", name);
            return 0.0;
        }

		AU_LOG_INFO("Unknown `GetValue` input requested: ", name);
        return 0.0;
    }

	bool Manager::ActiveCategory(const std::string &category)
	{
		bool changed = IManager::ActiveCategory(category);

		// TODO: Change this is future, because this cancels continuous button held in category transition
		// This fixed instant button call after category change
		// If you listening on same button in two different categories and then you
		// cahange category to the other, the callback will call the button in the set
		// category in the SAME TICK!
		// So this prevent that
		// (This was made when you want to lock and unlock cursor in two different categories)
		// (When this wasn't here, it randomly set the cursor and when I hold it, it flicker the cursor)
		if(changed)
            Clear();

		return changed;
	}

    double Manager::GetAnalog(const IManager::Action_t& action)
    {
        const auto it_Configurations = m_Configurations.find(GetActiveCategory());
        if(it_Configurations == m_Configurations.end())
            return 0;

        const auto& actions = it_Configurations->second;
        const auto it_Actions = actions.find(action);
        if(it_Actions == actions.end())
            return 0;

        const auto& keys = it_Actions->second;

        double value = 0;
        for(const auto& key : keys)
        {
            if(key.second)
                value -= GetValue(key.first);
            else
                value += GetValue(key.first);
        }
        return glm::clamp(value, -1.0, 1.0);
    }

    double Manager::GetAnalog_Prev(const IManager::Action_t& action)
    {
        const auto it_Configurations = m_Configurations.find(GetActiveCategory());
        if(it_Configurations == m_Configurations.end())
            return 0;

        const auto& actions = it_Configurations->second;
        const auto it_Actions = actions.find(action);
        if(it_Actions == actions.end())
            return 0;

        const auto& keys = it_Actions->second;

        double value = 0;
        for(const auto& key : keys)
        {
            if(key.second)
                value -= GetValue_Prev(key.first);
            else
                value += GetValue_Prev(key.first);
        }
        return glm::clamp(value, -1.0, 1.0);
    }
}
#endif