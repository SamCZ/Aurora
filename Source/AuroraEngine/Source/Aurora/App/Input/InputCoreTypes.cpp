#include "InputCoreTypes.hpp"

#include <utility>

namespace Aurora::App
{
    String Key::GetName() const
    {
        return KeyName;
    }

    bool Key::IsModifierKey() const
    {
        return KeyDetails->IsModifierKey();
    }

    bool Key::IsGamepadKey() const
    {
        return KeyDetails->IsGamepadKey();
    }

    bool Key::IsTouch() const
    {
        return KeyDetails->IsTouch();
    }

    bool Key::IsMouseButton() const
    {
        return KeyDetails->IsMouseButton();
    }

    bool Key::IsFloatAxis() const
    {
        return KeyDetails->IsFloatAxis();
    }

    bool Key::IsVectorAxis() const
    {
        return KeyDetails->IsVectorAxis();
    }

    bool Key::IsBindableInBlueprints() const
    {
        return KeyDetails->IsBindableInBlueprints();
    }

    bool Key::ShouldUpdateAxisWithoutSamples() const
    {
        return KeyDetails->ShouldUpdateAxisWithoutSamples();
    }

    KeyDetails::KeyDetails(Key InKey, String InLongDisplayName, uint8_t InKeyFlags, String InShortDisplayName)
            : m_Key(std::move(InKey))
            , LongDisplayName(std::move(InLongDisplayName))
            , ShortDisplayName(std::move(InShortDisplayName))
    {
        CommonInit(InKeyFlags);
    }

    KeyDetails::KeyDetails(Key InKey, String InLongDisplayName, String InShortDisplayName, uint8_t InKeyFlags)
            : m_Key(std::move(InKey))
            , LongDisplayName(std::move(InLongDisplayName))
            , ShortDisplayName(std::move(InShortDisplayName))
    {
        CommonInit(InKeyFlags);
    }

    void KeyDetails::CommonInit(const uint8_t InKeyFlags)
    {
        bIsModifierKey = ((InKeyFlags & KeyFlags::ModifierKey) != 0);
        bIsGamepadKey = ((InKeyFlags & KeyFlags::GamepadKey) != 0);
        bIsTouch = ((InKeyFlags & KeyFlags::Touch) != 0);
        bIsMouseButton = ((InKeyFlags & KeyFlags::MouseButton) != 0);
        bIsBindableInBlueprints = ((~InKeyFlags & KeyFlags::NotBlueprintBindableKey) != 0);
        bShouldUpdateAxisWithoutSamples = ((InKeyFlags & KeyFlags::UpdateAxisWithoutSamples) != 0);

        if ((InKeyFlags & KeyFlags::FloatAxis) != 0)
        {
            //ensure((InKeyFlags & KeyFlags::VectorAxis) == 0);
            AxisType = InputAxisType::Float;
        }
        else if ((InKeyFlags & KeyFlags::VectorAxis) != 0)
        {
            AxisType = InputAxisType::Vector;
        }
        else
        {
            AxisType = InputAxisType::None;
        }
    }
}
