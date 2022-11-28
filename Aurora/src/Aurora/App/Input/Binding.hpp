#pragma once
#include "Aurora/Core/Common.hpp"

#include <set>
#include <string>

namespace Aurora::Input
{
    AU_CLASS(Manager);
}

namespace Aurora::Input
{
	AU_CLASS(IManager);

	AU_CLASS(Binding)
    {
        friend class IManager;
        friend class Manager;

    protected:
        Binding(std::set<std::string> categories, std::string name, bool active)
                : m_Categories(std::move(categories)), m_InputName(std::move(name)), m_Active(active)
        {
        }
        [[nodiscard]] inline static Binding_ptr make_shared( const std::set<std::string>& categories, const std::string& name, bool active)
        {
            return Binding_ptr(new Binding(categories, name, active));
        }

    protected:
        std::set<std::string> m_Categories;
        std::string m_InputName;
    public:
        [[nodiscard]] inline const std::set<std::string>& Categories() const noexcept { return m_Categories; }
        [[nodiscard]] inline const          std::string&  InputName()  const noexcept { return m_InputName; }

    protected:
        double m_ValueCurrent = 0;
        double m_ValuePrevious = 0;
        bool m_Active;
        /// Axis: abs <= this is considered 0
        /// Button: value <= this is considered to be released
        /// Must be >= 0, otherwise won't work correctly.
        double m_DeadZone = 0.2;
        /// How long is this input held?
        /// Checks `m_ValueCurrent >= m_DeadZone`
        double m_HeldTime = 0;

    public:
        /// Whenever current Active Category is one of categories assigned for this mapping
        [[nodiscard]] inline bool IsActive() const noexcept { return m_Active; }

    public:
        [[nodiscard]] inline double RawValue()          const noexcept { return m_ValueCurrent; }
        [[nodiscard]] inline double RawValue_Previous() const noexcept { return m_ValuePrevious; }

    // Axis
    public:
        /// Axis: Value between -1 and 1
        /// Button: Value between 0 (released) and 1 (pressed)
        [[nodiscard]] inline double Value()          const noexcept { return std::abs(m_ValueCurrent) <= m_DeadZone ? 0.0 : std::clamp(m_ValueCurrent, -1.0, 1.0); }
        /// Same as `Value` but looks at previous tick
        [[nodiscard]] inline double Value_Previous() const noexcept { return std::abs(m_ValuePrevious) <= m_DeadZone ? 0.0 : std::clamp(m_ValuePrevious, -1.0, 1.0); }

    // Button
    public:
        /// Is the button currently held down?
        [[nodiscard]] inline bool   IsPressed()  const noexcept { return m_ValueCurrent > m_DeadZone; }
        /// Was the button held down last tick?
        [[nodiscard]] inline bool   WasPressed() const noexcept { return m_ValuePrevious > m_DeadZone; }
        /// Is current tick the one when the button was pressed?
        [[nodiscard]] inline bool   IsDown()     const noexcept { return IsPressed() && !WasPressed(); }
        /// Is current tick the one when the button was released?
        [[nodiscard]] inline bool   IsUp()       const noexcept { return !IsPressed() && WasPressed(); }
        [[nodiscard]] inline double HeldTime()   const noexcept { return m_HeldTime; };
    };
}
