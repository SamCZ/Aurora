#pragma once

#include <Aurora/Core/String.hpp>
#include <Aurora/Core/SmartPointer.hpp>
#include <utility>

namespace Aurora::App
{
    struct Key
    {
    private:
        String KeyName;
    public:

        Key() = default;

        explicit Key(String InName) : KeyName(std::move(InName))
        {
        }

        explicit Key(const char* InName) : KeyName(String(InName))
        {
        }

        bool IsModifierKey() const;
        bool IsGamepadKey() const;
        bool IsTouch() const;
        bool IsMouseButton() const;
        bool IsFloatAxis() const;
        bool IsVectorAxis() const;
        bool IsBindableInBlueprints() const;
        bool ShouldUpdateAxisWithoutSamples() const;
        String GetName() const;

        friend bool operator==(const Key& KeyA, const Key& KeyB) { return KeyA.KeyName == KeyB.KeyName; }
        friend bool operator!=(const Key& KeyA, const Key& KeyB) { return KeyA.KeyName != KeyB.KeyName; }
        friend bool operator<(const Key& KeyA, const Key& KeyB) { return KeyA.KeyName < KeyB.KeyName; }
        //friend uint32 GetTypeHash(const Key& Key) { return GetTypeHash(Key.KeyName); }

        friend struct EKeys;

        mutable SharedPtr<struct KeyDetails> KeyDetails;
    };

    struct KeyDetails
    {
    public:
        enum class InputAxisType : uint8_t
        {
            None,
            Float,
            Vector
        };

    private:
        Key m_Key;

        uint8_t bIsModifierKey : 1{};
        uint8_t bIsGamepadKey : 1{};
        uint8_t bIsTouch : 1{};
        uint8_t bIsMouseButton : 1{};
        uint8_t bIsBindableInBlueprints : 1{};
        uint8_t bShouldUpdateAxisWithoutSamples : 1{};

    public:
        InputAxisType AxisType;
        String LongDisplayName;
        String ShortDisplayName;
    public:
        enum KeyFlags : uint16_t
        {
            GamepadKey = 1u << 0u,
            Touch = 1 << 1,
            MouseButton = 1 << 2,
            ModifierKey = 1 << 3,
            NotBlueprintBindableKey = 1 << 4,
            FloatAxis = 1 << 5,
            VectorAxis = 1 << 6,
            UpdateAxisWithoutSamples = 1 << 7,

            NoFlags = 0,
        };

        KeyDetails(Key  InKey, String  InLongDisplayName, uint8_t InKeyFlags = 0, String  InShortDisplayName = String());
        KeyDetails(Key  InKey, String  InLongDisplayName, String  InShortDisplayName, uint8_t InKeyFlags = 0);

        inline bool IsModifierKey() const { return bIsModifierKey != 0; }
        inline bool IsGamepadKey() const { return bIsGamepadKey != 0; }
        inline bool IsTouch() const { return bIsTouch != 0; }
        inline bool IsMouseButton() const { return bIsMouseButton != 0; }
        inline bool IsFloatAxis() const { return AxisType == InputAxisType::Float; }
        inline bool IsVectorAxis() const { return AxisType == InputAxisType::Vector; }
        inline bool IsBindableInBlueprints() const { return bIsBindableInBlueprints != 0; }
        inline bool ShouldUpdateAxisWithoutSamples() const { return bShouldUpdateAxisWithoutSamples != 0; }
        inline const Key& GetKey() const { return m_Key; }

    private:
        void CommonInit(uint8_t InKeyFlags);
    };

    DEFINE_PTR(KeyDetails)
}