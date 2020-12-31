#include "Keys.h"

namespace Aurora::App
{
    const Key Keys::AnyKey("AnyKey");

    const Key Keys::MouseX("MouseX");
    const Key Keys::MouseY("MouseY");
    const Key Keys::MouseScrollUp("MouseScrollUp");
    const Key Keys::MouseScrollDown("MouseScrollDown");
    const Key Keys::MouseWheelAxis("MouseWheelAxis");

    const Key Keys::LeftMouseButton("LeftMouseButton");
    const Key Keys::RightMouseButton("RightMouseButton");
    const Key Keys::MiddleMouseButton("MiddleMouseButton");
    const Key Keys::ThumbMouseButton("ThumbMouseButton");
    const Key Keys::ThumbMouseButton2("ThumbMouseButton2");

    const Key Keys::BackSpace("BackSpace");
    const Key Keys::Tab("Tab");
    const Key Keys::Enter("Enter");
    const Key Keys::Pause("Pause");

    const Key Keys::CapsLock("CapsLock");
    const Key Keys::Escape("Escape");
    const Key Keys::SpaceBar("SpaceBar");
    const Key Keys::PageUp("PageUp");
    const Key Keys::PageDown("PageDown");
    const Key Keys::End("End");
    const Key Keys::Home("Home");

    const Key Keys::Left("Left");
    const Key Keys::Up("Up");
    const Key Keys::Right("Right");
    const Key Keys::Down("Down");

    const Key Keys::Insert("Insert");
    const Key Keys::Delete("Delete");

    const Key Keys::Zero("Zero");
    const Key Keys::One("One");
    const Key Keys::Two("Two");
    const Key Keys::Three("Three");
    const Key Keys::Four("Four");
    const Key Keys::Five("Five");
    const Key Keys::Six("Six");
    const Key Keys::Seven("Seven");
    const Key Keys::Eight("Eight");
    const Key Keys::Nine("Nine");

    const Key Keys::A("A");
    const Key Keys::B("B");
    const Key Keys::C("C");
    const Key Keys::D("D");
    const Key Keys::E("E");
    const Key Keys::F("F");
    const Key Keys::G("G");
    const Key Keys::H("H");
    const Key Keys::I("I");
    const Key Keys::J("J");
    const Key Keys::K("K");
    const Key Keys::L("L");
    const Key Keys::M("M");
    const Key Keys::N("N");
    const Key Keys::O("O");
    const Key Keys::P("P");
    const Key Keys::Q("Q");
    const Key Keys::R("R");
    const Key Keys::S("S");
    const Key Keys::T("T");
    const Key Keys::U("U");
    const Key Keys::V("V");
    const Key Keys::W("W");
    const Key Keys::X("X");
    const Key Keys::Y("Y");
    const Key Keys::Z("Z");

    const Key Keys::NumPadZero("NumPadZero");
    const Key Keys::NumPadOne("NumPadOne");
    const Key Keys::NumPadTwo("NumPadTwo");
    const Key Keys::NumPadThree("NumPadThree");
    const Key Keys::NumPadFour("NumPadFour");
    const Key Keys::NumPadFive("NumPadFive");
    const Key Keys::NumPadSix("NumPadSix");
    const Key Keys::NumPadSeven("NumPadSeven");
    const Key Keys::NumPadEight("NumPadEight");
    const Key Keys::NumPadNine("NumPadNine");

    const Key Keys::Multiply("Multiply");
    const Key Keys::Add("Add");
    const Key Keys::Subtract("Subtract");
    const Key Keys::Decimal("Decimal");
    const Key Keys::Divide("Divide");

    const Key Keys::F1("F1");
    const Key Keys::F2("F2");
    const Key Keys::F3("F3");
    const Key Keys::F4("F4");
    const Key Keys::F5("F5");
    const Key Keys::F6("F6");
    const Key Keys::F7("F7");
    const Key Keys::F8("F8");
    const Key Keys::F9("F9");
    const Key Keys::F10("F10");
    const Key Keys::F11("F11");
    const Key Keys::F12("F12");

    const Key Keys::NumLock("NumLock");

    const Key Keys::ScrollLock("ScrollLock");

    const Key Keys::LeftShift("LeftShift");
    const Key Keys::RightShift("RightShift");
    const Key Keys::LeftControl("LeftControl");
    const Key Keys::RightControl("RightControl");
    const Key Keys::LeftAlt("LeftAlt");
    const Key Keys::RightAlt("RightAlt");
    const Key Keys::LeftCommand("LeftCommand");
    const Key Keys::RightCommand("RightCommand");

    const Key Keys::Semicolon("Semicolon");
    const Key Keys::Equals("Equals");
    const Key Keys::Comma("Comma");
    const Key Keys::Underscore("Underscore");
    const Key Keys::Hyphen("Hyphen");
    const Key Keys::Period("Period");
    const Key Keys::Slash("Slash");
    const Key Keys::Tilde("Tilde");
    const Key Keys::LeftBracket("LeftBracket");
    const Key Keys::LeftParantheses("LeftParantheses");
    const Key Keys::Backslash("Backslash");
    const Key Keys::RightBracket("RightBracket");
    const Key Keys::RightParantheses("RightParantheses");
    const Key Keys::Apostrophe("Apostrophe");
    const Key Keys::Quote("Quote");

    const Key Keys::Asterix("Asterix");
    const Key Keys::Ampersand("Ampersand");
    const Key Keys::Caret("Caret");
    const Key Keys::Dollar("Dollar");
    const Key Keys::Exclamation("Exclamation");
    const Key Keys::Colon("Colon");

    const Key Keys::A_AccentGrave("A_AccentGrave");
    const Key Keys::E_AccentGrave("E_AccentGrave");
    const Key Keys::E_AccentAigu("E_AccentAigu");
    const Key Keys::C_Cedille("C_Cedille");
    const Key Keys::Section("Section");


// Setup platform specific keys
#if PLATFORM_MAC
    const Key Keys::Platform_Delete = Keys::BackSpace;
#else
    const Key Keys::Platform_Delete = Keys::Delete;
#endif

// Ensure that the Gamepad_ names match those in GenericApplication.cpp
    const Key Keys::Gamepad_LeftX("Gamepad_LeftX");
    const Key Keys::Gamepad_LeftY("Gamepad_LeftY");
    const Key Keys::Gamepad_RightX("Gamepad_RightX");
    const Key Keys::Gamepad_RightY("Gamepad_RightY");
    const Key Keys::Gamepad_LeftTriggerAxis("Gamepad_LeftTriggerAxis");
    const Key Keys::Gamepad_RightTriggerAxis("Gamepad_RightTriggerAxis");

    const Key Keys::Gamepad_LeftThumbstick("Gamepad_LeftThumbstick");
    const Key Keys::Gamepad_RightThumbstick("Gamepad_RightThumbstick");
    const Key Keys::Gamepad_Special_Left("Gamepad_Special_Left");
    const Key Keys::Gamepad_Special_Left_X("Gamepad_Special_Left_X");
    const Key Keys::Gamepad_Special_Left_Y("Gamepad_Special_Left_Y");
    const Key Keys::Gamepad_Special_Right("Gamepad_Special_Right");
    const Key Keys::Gamepad_FaceButton_Bottom("Gamepad_FaceButton_Bottom");
    const Key Keys::Gamepad_FaceButton_Right("Gamepad_FaceButton_Right");
    const Key Keys::Gamepad_FaceButton_Left("Gamepad_FaceButton_Left");
    const Key Keys::Gamepad_FaceButton_Top("Gamepad_FaceButton_Top");
    const Key Keys::Gamepad_LeftShoulder("Gamepad_LeftShoulder");
    const Key Keys::Gamepad_RightShoulder("Gamepad_RightShoulder");
    const Key Keys::Gamepad_LeftTrigger("Gamepad_LeftTrigger");
    const Key Keys::Gamepad_RightTrigger("Gamepad_RightTrigger");
    const Key Keys::Gamepad_DPad_Up("Gamepad_DPad_Up");
    const Key Keys::Gamepad_DPad_Down("Gamepad_DPad_Down");
    const Key Keys::Gamepad_DPad_Right("Gamepad_DPad_Right");
    const Key Keys::Gamepad_DPad_Left("Gamepad_DPad_Left");

// Virtual key codes used for input axis button press/release emulation
    const Key Keys::Gamepad_LeftStick_Up("Gamepad_LeftStick_Up");
    const Key Keys::Gamepad_LeftStick_Down("Gamepad_LeftStick_Down");
    const Key Keys::Gamepad_LeftStick_Right("Gamepad_LeftStick_Right");
    const Key Keys::Gamepad_LeftStick_Left("Gamepad_LeftStick_Left");

    const Key Keys::Gamepad_RightStick_Up("Gamepad_RightStick_Up");
    const Key Keys::Gamepad_RightStick_Down("Gamepad_RightStick_Down");
    const Key Keys::Gamepad_RightStick_Right("Gamepad_RightStick_Right");
    const Key Keys::Gamepad_RightStick_Left("Gamepad_RightStick_Left");

// const Key Keys::Vector axes (FVector("Vector axes (FVector"); not float)
    const Key Keys::Tilt("Tilt");
    const Key Keys::RotationRate("RotationRate");
    const Key Keys::Gravity("Gravity");
    const Key Keys::Acceleration("Acceleration");

    const Key Keys::Invalid("");

    Map<Key, KeyDetailsPtr> Keys::InputKeys;
    bool Keys::m_Initialized = false;

    void Keys::Initialize()
    {
        if (m_Initialized)
        {
            return;
        }

        m_Initialized = true;

#define LOCTEXT(first, second) first

        AddKey(KeyDetails(Keys::AnyKey, LOCTEXT("AnyKey", "Any Key")));

        AddKey(KeyDetails(Keys::MouseX, LOCTEXT("MouseX", "Mouse X"), KeyDetails::FloatAxis | KeyDetails::MouseButton | KeyDetails::UpdateAxisWithoutSamples));
        AddKey(KeyDetails(Keys::MouseY, LOCTEXT("MouseY", "Mouse Y"), KeyDetails::FloatAxis | KeyDetails::MouseButton | KeyDetails::UpdateAxisWithoutSamples));
        AddKey(KeyDetails(Keys::MouseWheelAxis, LOCTEXT("MouseWheelAxis", "Mouse Wheel Axis"), KeyDetails::FloatAxis | KeyDetails::MouseButton | KeyDetails::UpdateAxisWithoutSamples));
        AddKey(KeyDetails(Keys::MouseScrollUp, LOCTEXT("MouseScrollUp", "Mouse Wheel Up"), KeyDetails::MouseButton));
        AddKey(KeyDetails(Keys::MouseScrollDown, LOCTEXT("MouseScrollDown", "Mouse Wheel Down"), KeyDetails::MouseButton));

        AddKey(KeyDetails(Keys::LeftMouseButton, LOCTEXT("LeftMouseButton", "Left Mouse Button"), KeyDetails::MouseButton));
        AddKey(KeyDetails(Keys::RightMouseButton, LOCTEXT("RightMouseButton", "Right Mouse Button"), KeyDetails::MouseButton));
        AddKey(KeyDetails(Keys::MiddleMouseButton, LOCTEXT("MiddleMouseButton", "Middle Mouse Button"), KeyDetails::MouseButton));
        AddKey(KeyDetails(Keys::ThumbMouseButton, LOCTEXT("ThumbMouseButton", "Thumb Mouse Button"), KeyDetails::MouseButton));
        AddKey(KeyDetails(Keys::ThumbMouseButton2, LOCTEXT("ThumbMouseButton2", "Thumb Mouse Button 2"), KeyDetails::MouseButton));

        AddKey(KeyDetails(Keys::Tab, LOCTEXT("Tab", "Tab")));
        AddKey(KeyDetails(Keys::Enter, LOCTEXT("Enter", "Enter")));
        AddKey(KeyDetails(Keys::Pause, LOCTEXT("Pause", "Pause")));

        AddKey(KeyDetails(Keys::CapsLock, LOCTEXT("CapsLock", "Caps Lock"), LOCTEXT("CapsLockShort", "Caps")));
        AddKey(KeyDetails(Keys::Escape, LOCTEXT("Escape", "Escape"), LOCTEXT("EscapeShort", "Esc")));
        AddKey(KeyDetails(Keys::SpaceBar, LOCTEXT("SpaceBar", "Space Bar"), LOCTEXT("SpaceBarShort", "Space")));
        AddKey(KeyDetails(Keys::PageUp, LOCTEXT("PageUp", "Page Up"), LOCTEXT("PageUpShort", "PgUp")));
        AddKey(KeyDetails(Keys::PageDown, LOCTEXT("PageDown", "Page Down"), LOCTEXT("PageDownShort", "PgDn")));
        AddKey(KeyDetails(Keys::End, LOCTEXT("End", "End")));
        AddKey(KeyDetails(Keys::Home, LOCTEXT("Home", "Home")));

        AddKey(KeyDetails(Keys::Left, LOCTEXT("Left", "Left")));
        AddKey(KeyDetails(Keys::Up, LOCTEXT("Up", "Up")));
        AddKey(KeyDetails(Keys::Right, LOCTEXT("Right", "Right")));
        AddKey(KeyDetails(Keys::Down, LOCTEXT("Down", "Down")));

        AddKey(KeyDetails(Keys::Insert, LOCTEXT("Insert", "Insert"), LOCTEXT("InsertShort", "Ins")));

#if PLATFORM_MAC
        AddKey(KeyDetails(Keys::BackSpace, LOCTEXT("Delete", "Delete"), LOCTEXT("DeleteShort", "Del")));
	AddKey(KeyDetails(Keys::Delete, LOCTEXT("ForwardDelete", "Fn+Delete")));
#else
        AddKey(KeyDetails(Keys::BackSpace, LOCTEXT("BackSpace", "Backspace")));
        AddKey(KeyDetails(Keys::Delete, LOCTEXT("Delete", "Delete"), LOCTEXT("DeleteShort", "Del")));
#endif

        AddKey(KeyDetails(Keys::Zero, String("0")));
        AddKey(KeyDetails(Keys::One, String("1")));
        AddKey(KeyDetails(Keys::Two, String("2")));
        AddKey(KeyDetails(Keys::Three, String("3")));
        AddKey(KeyDetails(Keys::Four, String("4")));
        AddKey(KeyDetails(Keys::Five, String("5")));
        AddKey(KeyDetails(Keys::Six, String("6")));
        AddKey(KeyDetails(Keys::Seven, String("7")));
        AddKey(KeyDetails(Keys::Eight, String("8")));
        AddKey(KeyDetails(Keys::Nine, String("9")));

        AddKey(KeyDetails(Keys::A, String("A")));
        AddKey(KeyDetails(Keys::B, String("B")));
        AddKey(KeyDetails(Keys::C, String("C")));
        AddKey(KeyDetails(Keys::D, String("D")));
        AddKey(KeyDetails(Keys::E, String("E")));
        AddKey(KeyDetails(Keys::F, String("F")));
        AddKey(KeyDetails(Keys::G, String("G")));
        AddKey(KeyDetails(Keys::H, String("H")));
        AddKey(KeyDetails(Keys::I, String("I")));
        AddKey(KeyDetails(Keys::J, String("J")));
        AddKey(KeyDetails(Keys::K, String("K")));
        AddKey(KeyDetails(Keys::L, String("L")));
        AddKey(KeyDetails(Keys::M, String("M")));
        AddKey(KeyDetails(Keys::N, String("N")));
        AddKey(KeyDetails(Keys::O, String("O")));
        AddKey(KeyDetails(Keys::P, String("P")));
        AddKey(KeyDetails(Keys::Q, String("Q")));
        AddKey(KeyDetails(Keys::R, String("R")));
        AddKey(KeyDetails(Keys::S, String("S")));
        AddKey(KeyDetails(Keys::T, String("T")));
        AddKey(KeyDetails(Keys::U, String("U")));
        AddKey(KeyDetails(Keys::V, String("V")));
        AddKey(KeyDetails(Keys::W, String("W")));
        AddKey(KeyDetails(Keys::X, String("X")));
        AddKey(KeyDetails(Keys::Y, String("Y")));
        AddKey(KeyDetails(Keys::Z, String("Z")));

        AddKey(KeyDetails(Keys::NumPadZero, LOCTEXT("NumPadZero", "Num 0")));
        AddKey(KeyDetails(Keys::NumPadOne, LOCTEXT("NumPadOne", "Num 1")));
        AddKey(KeyDetails(Keys::NumPadTwo, LOCTEXT("NumPadTwo", "Num 2")));
        AddKey(KeyDetails(Keys::NumPadThree, LOCTEXT("NumPadThree", "Num 3")));
        AddKey(KeyDetails(Keys::NumPadFour, LOCTEXT("NumPadFour", "Num 4")));
        AddKey(KeyDetails(Keys::NumPadFive, LOCTEXT("NumPadFive", "Num 5")));
        AddKey(KeyDetails(Keys::NumPadSix, LOCTEXT("NumPadSix", "Num 6")));
        AddKey(KeyDetails(Keys::NumPadSeven, LOCTEXT("NumPadSeven", "Num 7")));
        AddKey(KeyDetails(Keys::NumPadEight, LOCTEXT("NumPadEight", "Num 8")));
        AddKey(KeyDetails(Keys::NumPadNine, LOCTEXT("NumPadNine", "Num 9")));

        AddKey(KeyDetails(Keys::Multiply, LOCTEXT("Multiply", "Num *")));
        AddKey(KeyDetails(Keys::Add, LOCTEXT("Add", "Num +")));
        AddKey(KeyDetails(Keys::Subtract, LOCTEXT("Subtract", "Num -")));
        AddKey(KeyDetails(Keys::Decimal, LOCTEXT("Decimal", "Num .")));
        AddKey(KeyDetails(Keys::Divide, LOCTEXT("Divide", "Num /")));

        AddKey(KeyDetails(Keys::F1, LOCTEXT("F1", "F1")));
        AddKey(KeyDetails(Keys::F2, LOCTEXT("F2", "F2")));
        AddKey(KeyDetails(Keys::F3, LOCTEXT("F3", "F3")));
        AddKey(KeyDetails(Keys::F4, LOCTEXT("F4", "F4")));
        AddKey(KeyDetails(Keys::F5, LOCTEXT("F5", "F5")));
        AddKey(KeyDetails(Keys::F6, LOCTEXT("F6", "F6")));
        AddKey(KeyDetails(Keys::F7, LOCTEXT("F7", "F7")));
        AddKey(KeyDetails(Keys::F8, LOCTEXT("F8", "F8")));
        AddKey(KeyDetails(Keys::F9, LOCTEXT("F9", "F9")));
        AddKey(KeyDetails(Keys::F10, LOCTEXT("F10", "F10")));
        AddKey(KeyDetails(Keys::F11, LOCTEXT("F11", "F11")));
        AddKey(KeyDetails(Keys::F12, LOCTEXT("F12", "F12")));

        AddKey(KeyDetails(Keys::NumLock, LOCTEXT("NumLock", "Num Lock")));
        AddKey(KeyDetails(Keys::ScrollLock, LOCTEXT("ScrollLock", "Scroll Lock")));

        AddKey(KeyDetails(Keys::LeftShift, LOCTEXT("LeftShift", "Left Shift"), KeyDetails::ModifierKey));
        AddKey(KeyDetails(Keys::RightShift, LOCTEXT("RightShift", "Right Shift"), KeyDetails::ModifierKey));
        AddKey(KeyDetails(Keys::LeftControl, LOCTEXT("LeftControl", "Left Ctrl"), KeyDetails::ModifierKey));
        AddKey(KeyDetails(Keys::RightControl, LOCTEXT("RightControl", "Right Ctrl"), KeyDetails::ModifierKey));
        AddKey(KeyDetails(Keys::LeftAlt, LOCTEXT("LeftAlt", "Left Alt"), KeyDetails::ModifierKey));
        AddKey(KeyDetails(Keys::RightAlt, LOCTEXT("RightAlt", "Right Alt"), KeyDetails::ModifierKey));
        AddKey(KeyDetails(Keys::LeftCommand, LOCTEXT("LeftCommand", "Left Cmd"), KeyDetails::ModifierKey));
        AddKey(KeyDetails(Keys::RightCommand, LOCTEXT("RightCommand", "Right Cmd"), KeyDetails::ModifierKey));

        AddKey(KeyDetails(Keys::Semicolon, LOCTEXT("Semicolon", "Semicolon"), String(";")));
        AddKey(KeyDetails(Keys::Equals, LOCTEXT("Equals", "Equals"), String("=")));
        AddKey(KeyDetails(Keys::Comma, LOCTEXT("Comma", "Comma"), String(",")));
        AddKey(KeyDetails(Keys::Hyphen, LOCTEXT("Hyphen", "Hyphen"), String("-")));
        AddKey(KeyDetails(Keys::Underscore, LOCTEXT("Underscore", "Underscore"), String("_")));
        AddKey(KeyDetails(Keys::Period, LOCTEXT("Period", "Period"), String(".")));
        AddKey(KeyDetails(Keys::Slash, LOCTEXT("Slash", "Slash"), String("/")));
        AddKey(KeyDetails(Keys::Tilde, String("`"))); // Yes this is not actually a tilde, it is a long, sad, and old story
        AddKey(KeyDetails(Keys::LeftBracket, LOCTEXT("LeftBracket", "Left Bracket"), String("[")));
        AddKey(KeyDetails(Keys::Backslash, LOCTEXT("Backslash", "Backslash"), String("\\")));
        AddKey(KeyDetails(Keys::RightBracket, LOCTEXT("RightBracket", "Right Bracket"), String("]")));
        AddKey(KeyDetails(Keys::Apostrophe, LOCTEXT("Apostrophe", "Apostrophe"), String("'")));
        AddKey(KeyDetails(Keys::Quote, LOCTEXT("Quote", "Quote"), String("\"")));

        AddKey(KeyDetails(Keys::LeftParantheses, LOCTEXT("LeftParantheses", "Left Parantheses"), String("(")));
        AddKey(KeyDetails(Keys::RightParantheses, LOCTEXT("RightParantheses", "Right Parantheses"), String(")")));
        AddKey(KeyDetails(Keys::Ampersand, LOCTEXT("Ampersand", "Ampersand"), String("&")));
        AddKey(KeyDetails(Keys::Asterix, LOCTEXT("Asterix", "Asterisk"), String("*")));
        AddKey(KeyDetails(Keys::Caret, LOCTEXT("Caret", "Caret"), String("^")));
        AddKey(KeyDetails(Keys::Dollar, LOCTEXT("Dollar", "Dollar"), String("$")));
        AddKey(KeyDetails(Keys::Exclamation, LOCTEXT("Exclamation", "Exclamation"), String("!")));
        AddKey(KeyDetails(Keys::Colon, LOCTEXT("Colon", "Colon"), String(":")));

        AddKey(KeyDetails(Keys::A_AccentGrave, String(Chr(224))));
        AddKey(KeyDetails(Keys::E_AccentGrave, String(Chr(232))));
        AddKey(KeyDetails(Keys::E_AccentAigu, String(Chr(233))));
        AddKey(KeyDetails(Keys::C_Cedille, String(Chr(231))));
        AddKey(KeyDetails(Keys::Section, String(Chr(167))));


        // Setup Gamepad keys
        AddKey(KeyDetails(Keys::Gamepad_LeftX, LOCTEXT("Gamepad_LeftX", "Gamepad Left Thumbstick X-Axis"), KeyDetails::GamepadKey | KeyDetails::FloatAxis));
        AddKey(KeyDetails(Keys::Gamepad_LeftY, LOCTEXT("Gamepad_LeftY", "Gamepad Left Thumbstick Y-Axis"), KeyDetails::GamepadKey | KeyDetails::FloatAxis));
        AddKey(KeyDetails(Keys::Gamepad_RightX, LOCTEXT("Gamepad_RightX", "Gamepad Right Thumbstick X-Axis"), KeyDetails::GamepadKey | KeyDetails::FloatAxis));
        AddKey(KeyDetails(Keys::Gamepad_RightY, LOCTEXT("Gamepad_RightY", "Gamepad Right Thumbstick Y-Axis"), KeyDetails::GamepadKey | KeyDetails::FloatAxis));

        AddKey(KeyDetails(Keys::Gamepad_DPad_Up, LOCTEXT("Gamepad_DPad_Up", "Gamepad D-pad Up"), KeyDetails::GamepadKey));
        AddKey(KeyDetails(Keys::Gamepad_DPad_Down, LOCTEXT("Gamepad_DPad_Down", "Gamepad D-pad Down"), KeyDetails::GamepadKey));
        AddKey(KeyDetails(Keys::Gamepad_DPad_Right, LOCTEXT("Gamepad_DPad_Right", "Gamepad D-pad Right"), KeyDetails::GamepadKey));
        AddKey(KeyDetails(Keys::Gamepad_DPad_Left, LOCTEXT("Gamepad_DPad_Left", "Gamepad D-pad Left"), KeyDetails::GamepadKey));

        // Virtual key codes used for input axis button press/release emulation
        AddKey(KeyDetails(Keys::Gamepad_LeftStick_Up, LOCTEXT("Gamepad_LeftStick_Up", "Gamepad Left Thumbstick Up"), KeyDetails::GamepadKey));
        AddKey(KeyDetails(Keys::Gamepad_LeftStick_Down, LOCTEXT("Gamepad_LeftStick_Down", "Gamepad Left Thumbstick Down"), KeyDetails::GamepadKey));
        AddKey(KeyDetails(Keys::Gamepad_LeftStick_Right, LOCTEXT("Gamepad_LeftStick_Right", "Gamepad Left Thumbstick Right"), KeyDetails::GamepadKey));
        AddKey(KeyDetails(Keys::Gamepad_LeftStick_Left, LOCTEXT("Gamepad_LeftStick_Left", "Gamepad Left Thumbstick Left"), KeyDetails::GamepadKey));

        AddKey(KeyDetails(Keys::Gamepad_RightStick_Up, LOCTEXT("Gamepad_RightStick_Up", "Gamepad Right Thumbstick Up"), KeyDetails::GamepadKey));
        AddKey(KeyDetails(Keys::Gamepad_RightStick_Down, LOCTEXT("Gamepad_RightStick_Down", "Gamepad Right Thumbstick Down"), KeyDetails::GamepadKey));
        AddKey(KeyDetails(Keys::Gamepad_RightStick_Right, LOCTEXT("Gamepad_RightStick_Right", "Gamepad Right Thumbstick Right"), KeyDetails::GamepadKey));
        AddKey(KeyDetails(Keys::Gamepad_RightStick_Left, LOCTEXT("Gamepad_RightStick_Left", "Gamepad Right Thumbstick Left"), KeyDetails::GamepadKey));

        AddKey(KeyDetails(Keys::Tilt, LOCTEXT("Tilt", "Tilt"), KeyDetails::VectorAxis, "Motion"));
        AddKey(KeyDetails(Keys::RotationRate, LOCTEXT("RotationRate", "Rotation Rate"), KeyDetails::VectorAxis, "Motion"));
        AddKey(KeyDetails(Keys::Gravity, LOCTEXT("Gravity", "Gravity"), KeyDetails::VectorAxis, "Motion"));
        AddKey(KeyDetails(Keys::Acceleration, LOCTEXT("Acceleration", "Acceleration"), KeyDetails::VectorAxis, "Motion"));
    }

    void Keys::AddKey(const KeyDetails & keyDetails)
    {
        const Key& Key = keyDetails.GetKey();
        Key.KeyDetails = New(KeyDetails, keyDetails);
        InputKeys[Key] = Key.KeyDetails;
    }

    void Keys::GetAllKeys(List<Key>& OutKeys)
    {
        ITER(InputKeys, it)
        {
            OutKeys.push_back(it->first);
        }
    }

    KeyDetailsPtr Keys::GetKeyDetails(const Key& key)
    {
        if (InputKeys.find(key) != InputKeys.end())
        {
            return InputKeys[key];
        }

        return KeyDetailsPtr();
    }
}