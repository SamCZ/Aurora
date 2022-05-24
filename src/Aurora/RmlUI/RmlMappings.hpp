#pragma once

#include <Aurora/RmlUI/RmlUI.hpp>
#include <GLFW/glfw3.h>

namespace Aurora
{
	const std::unordered_map<unsigned, uint16_t> RmlKeyMap {
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
}