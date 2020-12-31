#pragma once

#include "InputCoreTypes.hpp"

namespace Aurora::App
{
    struct Keys
    {
    private:
        static Map<Key, KeyDetailsPtr> InputKeys;
        static bool m_Initialized;
    public:
        static const Key AnyKey;

        static const Key MouseX;
        static const Key MouseY;
        static const Key MouseScrollUp;
        static const Key MouseScrollDown;
        static const Key MouseWheelAxis;

        static const Key LeftMouseButton;
        static const Key RightMouseButton;
        static const Key MiddleMouseButton;
        static const Key ThumbMouseButton;
        static const Key ThumbMouseButton2;

        static const Key BackSpace;
        static const Key Tab;
        static const Key Enter;
        static const Key Pause;

        static const Key CapsLock;
        static const Key Escape;
        static const Key SpaceBar;
        static const Key PageUp;
        static const Key PageDown;
        static const Key End;
        static const Key Home;

        static const Key Left;
        static const Key Up;
        static const Key Right;
        static const Key Down;

        static const Key Insert;
        static const Key Delete;

        static const Key Zero;
        static const Key One;
        static const Key Two;
        static const Key Three;
        static const Key Four;
        static const Key Five;
        static const Key Six;
        static const Key Seven;
        static const Key Eight;
        static const Key Nine;

        static const Key A;
        static const Key B;
        static const Key C;
        static const Key D;
        static const Key E;
        static const Key F;
        static const Key G;
        static const Key H;
        static const Key I;
        static const Key J;
        static const Key K;
        static const Key L;
        static const Key M;
        static const Key N;
        static const Key O;
        static const Key P;
        static const Key Q;
        static const Key R;
        static const Key S;
        static const Key T;
        static const Key U;
        static const Key V;
        static const Key W;
        static const Key X;
        static const Key Y;
        static const Key Z;

        static const Key NumPadZero;
        static const Key NumPadOne;
        static const Key NumPadTwo;
        static const Key NumPadThree;
        static const Key NumPadFour;
        static const Key NumPadFive;
        static const Key NumPadSix;
        static const Key NumPadSeven;
        static const Key NumPadEight;
        static const Key NumPadNine;

        static const Key Multiply;
        static const Key Add;
        static const Key Subtract;
        static const Key Decimal;
        static const Key Divide;

        static const Key F1;
        static const Key F2;
        static const Key F3;
        static const Key F4;
        static const Key F5;
        static const Key F6;
        static const Key F7;
        static const Key F8;
        static const Key F9;
        static const Key F10;
        static const Key F11;
        static const Key F12;

        static const Key NumLock;

        static const Key ScrollLock;

        static const Key LeftShift;
        static const Key RightShift;
        static const Key LeftControl;
        static const Key RightControl;
        static const Key LeftAlt;
        static const Key RightAlt;
        static const Key LeftCommand;
        static const Key RightCommand;

        static const Key Semicolon;
        static const Key Equals;
        static const Key Comma;
        static const Key Underscore;
        static const Key Hyphen;
        static const Key Period;
        static const Key Slash;
        static const Key Tilde;
        static const Key LeftBracket;
        static const Key Backslash;
        static const Key RightBracket;
        static const Key Apostrophe;

        static const Key Ampersand;
        static const Key Asterix;
        static const Key Caret;
        static const Key Colon;
        static const Key Dollar;
        static const Key Exclamation;
        static const Key LeftParantheses;
        static const Key RightParantheses;
        static const Key Quote;

        static const Key A_AccentGrave;
        static const Key E_AccentGrave;
        static const Key E_AccentAigu;
        static const Key C_Cedille;
        static const Key Section;

        // Platform Keys
        // These keys platform specific versions of keys that go by different names.
        // The delete key is a good example, on Windows Delete is the virtual key for Delete.
        // On Macs, the Delete key is the virtual key for BackSpace.
        static const Key Platform_Delete;

        // Gamepad Keys
        static const Key Gamepad_LeftX;
        static const Key Gamepad_LeftY;
        static const Key Gamepad_RightX;
        static const Key Gamepad_RightY;
        static const Key Gamepad_LeftTriggerAxis;
        static const Key Gamepad_RightTriggerAxis;

        static const Key Gamepad_LeftThumbstick;
        static const Key Gamepad_RightThumbstick;
        static const Key Gamepad_Special_Left;
        static const Key Gamepad_Special_Left_X;
        static const Key Gamepad_Special_Left_Y;
        static const Key Gamepad_Special_Right;
        static const Key Gamepad_FaceButton_Bottom;
        static const Key Gamepad_FaceButton_Right;
        static const Key Gamepad_FaceButton_Left;
        static const Key Gamepad_FaceButton_Top;
        static const Key Gamepad_LeftShoulder;
        static const Key Gamepad_RightShoulder;
        static const Key Gamepad_LeftTrigger;
        static const Key Gamepad_RightTrigger;
        static const Key Gamepad_DPad_Up;
        static const Key Gamepad_DPad_Down;
        static const Key Gamepad_DPad_Right;
        static const Key Gamepad_DPad_Left;

        // Virtual key codes used for input axis button press/release emulation
        static const Key Gamepad_LeftStick_Up;
        static const Key Gamepad_LeftStick_Down;
        static const Key Gamepad_LeftStick_Right;
        static const Key Gamepad_LeftStick_Left;

        static const Key Gamepad_RightStick_Up;
        static const Key Gamepad_RightStick_Down;
        static const Key Gamepad_RightStick_Right;
        static const Key Gamepad_RightStick_Left;

        // static const Key Vector axes (Vector; not float)
        static const Key Tilt;
        static const Key RotationRate;
        static const Key Gravity;
        static const Key Acceleration;

        static const Key Invalid;

        static void Initialize();
        static void AddKey(const KeyDetails& KeyDetails);
        static void GetAllKeys(List<Key>& OutKeys);
        static KeyDetailsPtr GetKeyDetails(const Key& key);
    };
}

