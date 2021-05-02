#include "Manager.hpp"

#include <iostream>
#include <fstream>

namespace Aurora::Input
{
#ifdef AU_INPUT_GAMEPAD_VISUAL_FORCE
#   define FUNC_TRY_INPUT_TYPE_GAMEPAD()
#else
#   define FUNC_TRY_INPUT_TYPE_GAMEPAD() \
    /* Too long from last Gamepad input -> switch to Keyboard+Mouse */\
    if(m_TimeFromLastGamepadInput >= MaxTimeFromLastGamepadInput && InputType_IsGamepad(CurrentInputType()))\
    {\
        CurrentInputType(InputType::KeyboardAndMouse);\
    }
#endif

    void Manager::OnKeyChange(int keyCode, int scanCode, bool down)
    {
        if(keyCode != GLFW_KEY_UNKNOWN && keyCode >= 0)
        {
            if(keyCode >= MaxKeyCount) [[unlikely]]
            {
                AU_DEBUG_CERR("Keycode " << keyCode << " is too high");
                return;
            }
            m_KeyCodes[keyCode] = down;
            AU_DEBUG_COUT_INPUT("keyboard_code_" << keyCode, (down ? "down" : "up"));
        }
        else if(scanCode != GLFW_KEY_UNKNOWN && scanCode >= 0)
        {
            if(scanCode >= MaxKeyCount) [[unlikely]]
            {
                AU_DEBUG_CERR("Keycode " << keyCode << " is too high");
                return;
            }
            m_ScanCodes[scanCode] = down;
            AU_DEBUG_COUT_INPUT("keyboard_scancode_" << keyCode, (down ? "down" : "up"));
        }
        else
        {
            AU_DEBUG_CERR("OnKeyChange with no keyCode and scanCode, why?");
            return;
        }

        FUNC_TRY_INPUT_TYPE_GAMEPAD();
    }

    void Manager::OnMouseMove(const glm::dvec2& newPosition)
    {
        bool cursorLocked = IsCursorLocked();
        auto windowSize = m_GlfwWindow->GetSize();

        if(cursorLocked)
        {
            // Hidden
            glm::dvec2 change = newPosition - m_CursorPosition_Prev;
            m_CursorChange_Pixels = change * CursorSensitivity();
            m_CursorChange_Percentage = { change.x / windowSize.x, change.y / windowSize.y };

            // Visible
            m_CursorPosition_Pixels = newPosition;
            m_CursorPosition_Percentage = { newPosition.x / windowSize.x, newPosition.y / windowSize.y };
        }
        else // !cursorLocked
        {
            // Hidden
            m_CursorChange_Pixels = {0, 0};
            m_CursorChange_Percentage = {0, 0};

            // Visible
            m_CursorPosition_Pixels = newPosition;
            m_CursorPosition_Percentage = { newPosition.x / windowSize.x, newPosition.y / windowSize.y };
        }
        m_CursorPosition_Tmp = newPosition;

        AU_DEBUG_COUT_INPUT("mouse_x", m_CursorChange_Pixels.x);
        AU_DEBUG_COUT_INPUT("mouse_y", m_CursorChange_Pixels.y);
    }

    void Manager::OnJoystickConnectChange(Manager::JoystickIndex_t joyIndex, bool connected)
    {
        static_assert(GLFW_JOYSTICK_1  == 0);
        static_assert(GLFW_JOYSTICK_2  == 1);
        static_assert(GLFW_JOYSTICK_3  == 2);
        static_assert(GLFW_JOYSTICK_4  == 3);
        static_assert(GLFW_JOYSTICK_5  == 4);
        static_assert(GLFW_JOYSTICK_6  == 5);
        static_assert(GLFW_JOYSTICK_7  == 6);
        static_assert(GLFW_JOYSTICK_8  == 7);
        static_assert(GLFW_JOYSTICK_9  == 8);
        static_assert(GLFW_JOYSTICK_10 == 9);
        static_assert(GLFW_JOYSTICK_11 == 10);
        static_assert(GLFW_JOYSTICK_12 == 11);
        static_assert(GLFW_JOYSTICK_13 == 12);
        static_assert(GLFW_JOYSTICK_14 == 13);
        static_assert(GLFW_JOYSTICK_15 == 14);
        static_assert(GLFW_JOYSTICK_16 == 15);
        static_assert(GLFW_JOYSTICK_LAST == GLFW_JOYSTICK_16);

        static_assert(MaxJoystickCount >= 0);
        static_assert(GLFW_JOYSTICK_LAST < MaxJoystickCount); // last index < count

        if(joyIndex < 0 || joyIndex >= MaxJoystickCount)
        {
            AU_DEBUG_COUT("Joystick at index " << joyIndex << (connected ? "connected" : "disconnected") << " but index is not valid");
            return;
        }

#ifdef DEBUG
        if(connected)
            AU_DEBUG_COUT("Joystick " << joyIndex << " " << "connected and " << (glfwJoystickIsGamepad(joyIndex) ? "has a mapping" : "is not a valid gamepad"));
        else
            AU_DEBUG_COUT("Joystick " << joyIndex << " " << "disconnected");
#endif

        if(connected)
        {
            // Same as if gamepad button was pressed
            // Will switch `CurrentInputType` to correct gamepad
            m_TimeFromLastGamepadInput = 0;
        }
        else
        {
#ifndef AU_INPUT_GAMEPAD_VISUAL_FORCE
            bool hasGamepad = false;
            for(JoystickIndex_t ji = 0; ji < MaxJoystickCount; ji++)
            {
                if(!glfwJoystickIsGamepad(ji))
                    continue;
                hasGamepad = true;
                break;
            }
            if(!hasGamepad)
            {
                m_TimeFromLastGamepadInput = MaxTimeFromLastGamepadInput;
                FUNC_TRY_INPUT_TYPE_GAMEPAD();
            }
#endif
        }
    }

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
                    if(keyCode < m_KeyCodes.size())
                        return m_KeyCodes[keyCode];
                    else
                        return false;
                }

                if(scanCode != GLFW_KEY_UNKNOWN)
                {
                    if(scanCode < m_ScanCodes.size())
                        return m_ScanCodes[scanCode];
                    else
                        return false;
                }

                AU_DEBUG_CERR("Unknown `IsPressed` key requested: " << key);
                return false;
            }
        }

        // Mouse
        if(key.starts_with("mouse_"))
        {
            try
            {
                int index = std::stoi(key.data() + 6);
                if(index < m_MouseButtons.size())
                    return m_MouseButtons[index];
                else
                    return false;
            }
            catch(...)
            {
            }

            AU_DEBUG_CERR("Unknown `IsPressed` mouse button requested: " << key);
            return false;
        }

        // Gamepad
        if(key.starts_with("gamepad_"))
        {
            throw std::runtime_error("Gamepad not implemented");
        }

        return false;
    }

    void Manager::Update(double delta)
    {
        m_TimeFromLastGamepadInput += delta;
        for(JoystickIndex_t i = 0; i < MaxJoystickCount; i++)
        {
#ifdef AU_DEBUG_INPUT
            auto old = m_Gamepads[i];
#endif
            if(glfwGetGamepadState(i, &m_Gamepads[i]))
            {
                for(std::size_t bi = 0; bi <= GLFW_GAMEPAD_BUTTON_LAST; bi++)
                {
                    if(m_Gamepads[i].buttons[bi])
                    {
                        m_TimeFromLastGamepadInput = 0;
                        break;
                    }
                }
            }
            else
                m_Gamepads[i] = {};
#ifdef AU_DEBUG_INPUT
#   define AU_DEBUG_COUT_INPUT_GAMEPAD(index, buttonName, glfwButton) \
            if(old.buttons[glfwButton] != m_Gamepads[index].buttons[glfwButton]) \
                AU_DEBUG_COUT("Gamepad " << static_cast<int>(index) << ", " << buttonName << ": " << static_cast<bool>(m_Gamepads[index].buttons[glfwButton]));
#   define AU_DEBUG_COUT_INPUT_GAMEPAD_AXIS(index, axisName, glfwAxis) \
            if(old.axes[glfwAxis] != m_Gamepads[index].axes[glfwAxis]) \
                AU_DEBUG_COUT("Gamepad " << static_cast<int>(index) << ", " << axisName << ": " << static_cast<double>(m_Gamepads[index].axes[glfwAxis]));

            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "a", GLFW_GAMEPAD_BUTTON_A);
            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "b", GLFW_GAMEPAD_BUTTON_B);
            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "x", GLFW_GAMEPAD_BUTTON_X);
            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "y", GLFW_GAMEPAD_BUTTON_Y);

            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "back", GLFW_GAMEPAD_BUTTON_BACK);
            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "forward", GLFW_GAMEPAD_BUTTON_START);

            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "l1", GLFW_GAMEPAD_BUTTON_LEFT_BUMPER);
            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "r1", GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER);

            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "dpad_up", GLFW_GAMEPAD_BUTTON_DPAD_UP);
            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "dpad_down", GLFW_GAMEPAD_BUTTON_DPAD_DOWN);
            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "dpad_left", GLFW_GAMEPAD_BUTTON_DPAD_LEFT);
            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "dpad_right", GLFW_GAMEPAD_BUTTON_DPAD_RIGHT);

            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "stick_left", GLFW_GAMEPAD_BUTTON_LEFT_THUMB);
            AU_DEBUG_COUT_INPUT_GAMEPAD(i, "stick_right", GLFW_GAMEPAD_BUTTON_RIGHT_THUMB);

            AU_DEBUG_COUT_INPUT_GAMEPAD_AXIS(i, "stick_left_x", GLFW_GAMEPAD_AXIS_LEFT_X);
            AU_DEBUG_COUT_INPUT_GAMEPAD_AXIS(i, "stick_left_y", GLFW_GAMEPAD_AXIS_LEFT_Y);

            AU_DEBUG_COUT_INPUT_GAMEPAD_AXIS(i, "stick_right_x", GLFW_GAMEPAD_AXIS_RIGHT_X);
            AU_DEBUG_COUT_INPUT_GAMEPAD_AXIS(i, "stick_right_y", GLFW_GAMEPAD_AXIS_RIGHT_Y);

            AU_DEBUG_COUT_INPUT_GAMEPAD_AXIS(i, "l2", GLFW_GAMEPAD_AXIS_LEFT_TRIGGER);
            AU_DEBUG_COUT_INPUT_GAMEPAD_AXIS(i, "r2", GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);
#endif
        }
        if(m_TimeFromLastGamepadInput > 0 && m_TimeFromLastGamepadInput < MaxTimeFromLastGamepadInput && !InputType_IsGamepad(CurrentInputType()))
        {
            CurrentInputType(IsPictogramControllerConnected() ? InputType::Gamepad_Pictogram : InputType::Gamepad_ABXY);
        }

        auto itConfigs = m_Configurations.find(GetActiveCategory());
        if(itConfigs == m_Configurations.end())
        {
            // No active actions
        }
        else // itConfigs != m_Configurations.end()
        {
            const auto& currentActions = itConfigs->second;

            /// all used keys
            std::set<Key_t> keys;
            for(const auto& ca : currentActions)
                for(const auto& keyInvert : ca.second)
                    keys.emplace(keyInvert.first);

            std::unordered_map<Key_t, double> currentValues;
            double anyInputValue = 0;
            for(const Key_t& key : keys)
            {
                double val = GetValue(key);
                currentValues[key] = val;
                anyInputValue += val;
            }

            for(const Binding_ptr& binding : m_KnownBindings)
            {
                if(!binding->m_Active)
                    continue;

                binding->m_ValuePrevious = binding->m_ValueCurrent;

                if(binding->m_InputName.empty()) // any input
                {
                    binding->m_ValueCurrent = anyInputValue;
                    binding->m_HeldTime = 0;
                }
                else
                {
                    auto itCurrentAction = currentActions.find(binding->m_InputName);
                    if(itCurrentAction == currentActions.end()) // No config for the action
                    {
                        AU_DEBUG_CERR("No configuration for action " << binding->m_InputName);
                        continue;
                    }

                    double value = 0;

                    for(const auto& keyInvert : itCurrentAction->second)
                    {
                        auto itKey = currentValues.find(keyInvert.first);
                        value += itKey == currentValues.end() ? 0.0 : (itKey->second * (keyInvert.second ? -1 : 1));
                    }

                    if(std::abs(binding->m_ValueCurrent) <= binding->m_DeadZone)
                        binding->m_HeldTime = 0;

                    binding->m_ValueCurrent = value;

                    if(std::abs(binding->m_ValueCurrent) > binding->m_DeadZone)
                        binding->m_HeldTime += delta;
                }
            }
        }
    }

    double Manager::GetValue(const IManager::Key_t& name)
    {
        // Keyboard
        {
            int keyCode, scanCode;
            if(GetGlfwKey_Keyboard(name, &keyCode, &scanCode))
            {
                if(keyCode != GLFW_KEY_UNKNOWN)
                {
                    if(keyCode < m_KeyCodes.size())
                        return m_KeyCodes[keyCode] ? 1.0 : 0.0;
                    else
                        return 0.0;
                }

                if(scanCode != GLFW_KEY_UNKNOWN)
                {
                    if(scanCode < m_ScanCodes.size())
                        return m_ScanCodes[scanCode] ? 1.0 : 0.0;
                    else
                        return 0.0;
                }

                AU_DEBUG_CERR("Unknown `GetValue` key requested: " << name);
                return 0.0;
            }
        }

        // Mouse
        if(name.starts_with("mouse_"))
        {
            if(name == "mouse_x")
                return m_CursorChange_Pixels.x;
            if(name == "mouse_y")
                return m_CursorChange_Pixels.y;

            if(name == "mouse_wheel")
                return m_ScrollWheelChange.y;
            if(name == "mouse_wheel_side")
                return m_ScrollWheelChange.x;

            try
            {
                int index = std::stoi(name.data() + 6);
                if(index < m_MouseButtons.size())
                    return m_MouseButtons[index] ? 1.0 : 0.0;
                else
                    return 0.0;
            }
            catch(...)
            {
            }

            AU_DEBUG_CERR("Unknown `GetValue` mouse button/axis requested: " << name);
            return 0.0;
        }

        // Gamepad
        if(name.starts_with("gamepad_"))
        {
            static const std::size_t indexStart = 8;
            if(name.size() < indexStart + 2)
                return 0.0; // Not enough characters for `gamepad_*_`

            std::size_t secondIndex = name.find('_', indexStart);
            if(secondIndex == std::string::npos)
                return 0.0; // No second underscore

            if(name.size() <= secondIndex + 1)
                return 0.0;
            std::string suffix = name.substr(secondIndex + 1);
            if(suffix.empty())
                return 0.0;

            int joyIndex;
            try
            {
                joyIndex = std::stol(name.substr(indexStart, secondIndex - indexStart));
            }
            catch(...)
            {
                return 0.0;
            }
            if(joyIndex < 0 || joyIndex >= MaxJoystickCount)
                return 0.0;

            auto& joyState = m_Gamepads[joyIndex];

            if(suffix.starts_with("stick_"))
            {
                if(suffix == "stick_left")
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB] ? 1.0 : 0.0;
                }
                else if(suffix == "stick_right")
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB] ? 1.0 : 0.0;
                }
                else if(suffix == "stick_left_x")
                {
                    return joyState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
                }
                else if(suffix == "stick_left_y")
                {
                    return joyState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
                }
                else if(suffix == "stick_right_x")
                {
                    return joyState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
                }
                else if(suffix == "stick_right_y")
                {
                    return joyState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
                }
                else
                    return 0.0;
            }
            else if(suffix.starts_with("dpad_"))
            {
                if(suffix == "dpad_up")
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] ? 1.0 : 0.0;
                }
                else if(suffix == "dpad_down")
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] ? 1.0 : 0.0;
                }
                else if(suffix == "dpad_left")
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] ? 1.0 : 0.0;
                }
                else if(suffix == "dpad_right")
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] ? 1.0 : 0.0;
                }
                else
                    return 0.0;
            }
            else if(suffix[0] == 'l')
            {
                if(suffix.size() != 2)
                    return 0.0;

                if(suffix[1] == '1')
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] ? 1.0 : 0.0;
                }
                else if(suffix[1] == '2')
                {
                    return joyState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
                }
                else
                    return 0.0;
            }
            else if(suffix[0] == 'r')
            {
                if(suffix.size() != 2)
                    return 0.0;

                if(suffix[1] == '1')
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] ? 1.0 : 0.0;
                }
                else if(suffix[1] == '2')
                {
                    return joyState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
                }
                else
                    return 0.0;
            }
            else if(suffix.size() == 1)
            {
                if(suffix[0] == 'a')
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_A] ? 1.0 : 0.0;
                }
                else if(suffix[0] == 'b')
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_B] ? 1.0 : 0.0;
                }
                else if(suffix[0] == 'x')
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_X] ? 1.0 : 0.0;
                }
                else if(suffix[0] == 'y')
                {
                    return joyState.buttons[GLFW_GAMEPAD_BUTTON_Y] ? 1.0 : 0.0;
                }
                else
                    return 0.0;
            }
            else if(suffix == "forward") // start
            {
                return joyState.buttons[GLFW_GAMEPAD_BUTTON_START] ? 1.0 : 0.0;
            }
            else if(suffix == "back") // select
            {
                return joyState.buttons[GLFW_GAMEPAD_BUTTON_BACK] ? 1.0 : 0.0;
            }
            else if(suffix == "guide") // home
            {
                return joyState.buttons[GLFW_GAMEPAD_BUTTON_GUIDE] ? 1.0 : 0.0;
            }
            else
            {
                AU_DEBUG_CERR("Unknown `GetValue` gamepad button/axis requested: " << name);
                return 0.0;
            }
        }

        AU_DEBUG_CERR("Unknown `GetValue` input requested: " << name);
        return 0.0;
    }

    std::u8string Manager::GetKeyDisplayName(const IManager::Key_t& key)
    {
        throw std::runtime_error("Not Implemented");
    }

    std::u8string Manager::GetValueName(const IManager::Key_t& name)
    {
        throw std::runtime_error("Not Implemented");
    }

    void Manager::LoadGamepadConfig(const char* mappingContent)
    {
        if(mappingContent == nullptr)
            return;
        glfwUpdateGamepadMappings(mappingContent);
    }

    void Manager::LoadGamepadConfig(std::istream& in)
    {
        std::ostringstream oss;
        oss << in.rdbuf();

        LoadGamepadConfig(oss.str());
    }

    void Manager::LoadGamepadConfig(const std::filesystem::path& mappingFile)
    {
        if(!std::filesystem::exists(mappingFile) || !std::filesystem::is_regular_file(mappingFile))
            throw std::runtime_error("File Not Found (or is not a file)");
#ifdef DEBUG
        if(mappingFile.filename() != "gamecontrollerdb.txt")
            AU_DEBUG_CERR("Requested loading of Gamepad Config from file '" << mappingFile.filename() << "' but the file should be named 'gamecontrollerdb.txt'");
#endif

        std::fstream in(mappingFile, std::ios_base::in);
        if(!in.good())
            throw std::runtime_error("File Read Problem");

        LoadGamepadConfig(in);
    }

    bool Manager::IsPictogramControllerConnected()
    {
        for(JoystickIndex_t ji = 0; ji < MaxJoystickCount; ji++)
        {
            if(!glfwJoystickIsGamepad(ji))
                continue;
            const std::string name = glfwGetGamepadName(ji);
            if(name.starts_with("PS ") || name.find(" PS ") != std::string::npos)
                return true;
            if(name.starts_with("PlayStation ") || name.find(" PlayStation ") != std::string::npos)
                return true;
            if(name.starts_with("Play Station ") || name.find(" Play Station ") != std::string::npos)
                return true;
        }
        return false;
    }

	bool Manager::ActiveCategory(const std::string &category)
	{
		bool changed = IManager::ActiveCategory(category);

		// TODO: Change this is future, because this cancles continuous button held in cotegory transition
		// This fixed instant button call after category change
		// If you listening on same button in two different categories and then you
		// cahange category to the other, the callback will call the button in the set
		// category in the SAME TICK!
		// So this prevent that
		// (This was made when you want to lock and unlock cursor in two different categories)
		// (When this wasn't here, it randomly set the cursor and when I hold it, it flicker the cursor)
		if(changed) {
			std::fill(m_KeyCodes.begin(), m_KeyCodes.end(), false);
			std::fill(m_ScanCodes.begin(), m_ScanCodes.end(), false);
			std::fill(m_MouseButtons.begin(), m_MouseButtons.end(), false);
			std::fill(m_Gamepads.begin(), m_Gamepads.end(), GLFWgamepadstate{});
		}

		return changed;
	}

}

