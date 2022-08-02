#pragma once
#include "Aurora/Core/Common.hpp"

#include <set>
#include <map>
#include <string>
#include <filesystem>
#include <optional>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "Binding.hpp"

namespace Aurora::Input
{
    enum class InputType : uint8_t
    {
        KeyboardAndMouse  = 0,
        Gamepad_ABXY      = 1, ///< Nintendo, XBox
        Gamepad_Pictogram = 2  ///< PlayStation
    };
    inline static bool InputType_IsGamepad(InputType type) noexcept
    {
        switch(type)
        {
            case InputType::Gamepad_ABXY:
            case InputType::Gamepad_Pictogram:
                return true;
            default:
                return false;
        }
    }
    inline static std::string to_string(InputType type)
    {
        switch(type)
        {
            case InputType::KeyboardAndMouse:
                return "keyboard_and_mouse";
            case InputType::Gamepad_ABXY:
                return "gamepad_abxy";
            case InputType::Gamepad_Pictogram:
                return "gamepad_pictograms";
            default:
                throw std::runtime_error("Invalid value");
        }
    }

    AU_CLASS(IManager)
    {
    protected:
        IManager() = default;
        virtual ~IManager() = default;

    // Configuration
    public:
        typedef std::string Key_t;
        typedef std::string Action_t;
        typedef std::string Category_t;
        typedef std::map<Action_t, std::map<Key_t, bool>> Actions_t;
    protected:
        std::map<Category_t, Actions_t> m_Configurations{};
    public:
        void LoadConfig_JSON(const nlohmann::json& jConfig);
        void LoadConfig_JSON(const std::filesystem::path& configFile);

    // Bindings
    protected:
        std::set<Binding_ptr> m_KnownBindings{};
    public:
        [[nodiscard]] Binding_ptr Binding(      std::set<Category_t>  categories, const Action_t& action);
        [[nodiscard]] Binding_ptr Binding(const          Category_t & category,   const Action_t& action) { return Binding(std::set<Category_t>({category}), action); }
        [[nodiscard]] Binding_ptr Binding(const std::set<Category_t>& categories)                         { return Binding(categories, "*"); }
        [[nodiscard]] Binding_ptr Binding(const          Category_t & category)                           { return Binding(category, "*"); }

        // Input Type
    private:
        InputType                m_InputType = InputType::KeyboardAndMouse;
        std::optional<InputType> m_LockedInputType;
    public:
        /// What UI to show.
        /// Depends whenever controller input was triggered recently.
        [[nodiscard]] inline const InputType&                CurrentInputType()                              const noexcept { return m_LockedInputType.has_value() ? m_LockedInputType.value() : m_InputType; }
                                   void                      CurrentInputType(InputType value)                     noexcept;
        [[nodiscard]] inline const std::optional<InputType>& LockedInputType()                               const noexcept { return m_LockedInputType; }
                                   void                      LockedInputType(std::optional<InputType> value)       noexcept;
        [[nodiscard]] inline       bool                      IsInputTypeLocked()                             const noexcept { return m_LockedInputType.has_value(); }

    // Active category
    private:
        std::string m_ActiveCategory = "loading";
    public:
        [[nodiscard]] inline const std::string& GetActiveCategory() const noexcept { return m_ActiveCategory; }
        virtual bool ActiveCategory(const std::string& category);

    // Cursor
    public:
        [[nodiscard]] virtual const std::optional<glm::dvec2>& CursorPosition_Pixels()     const noexcept = 0;
        [[nodiscard]] virtual const               glm::dvec2 & CursorChange_Pixels()       const noexcept = 0;
        [[nodiscard]] virtual const std::optional<glm::dvec2>& CursorPosition_Percentage() const noexcept = 0;
        [[nodiscard]] virtual const               glm::dvec2 & CursorChange_Percentage()   const noexcept = 0;
        [[nodiscard]] virtual                     bool         IsCursorLocked()            const = 0;
                      virtual                     void         LockCursor(bool locked)     const = 0;
                      inline                      void         LockCursor()                const { LockCursor(true);  }
                      inline                      void         UnlockCursor()              const { LockCursor(false); }
    private:
        double m_CursorSensitivity = 1.0;
    public:
        [[nodiscard]] inline double CursorSensitivity()             const noexcept { return m_CursorSensitivity;         }
        inline void   CursorSensitivity(double value)       noexcept {        m_CursorSensitivity = value; }

        // Keys
    public:
        [[nodiscard]] virtual bool          IsPressed        (const Key_t& key) = 0;
        [[nodiscard]] virtual std::u8string GetKeyDisplayName(const Key_t& key) = 0;

        // Axis
    public:
        [[nodiscard]] virtual double        GetValue        (const Key_t& name) = 0;
        [[nodiscard]] virtual std::u8string GetValueName    (const Key_t& name) = 0;

    public:
    	virtual const std::vector<char8_t>& GetTypedChars() = 0;
    //TODO Scroll
    };
}
