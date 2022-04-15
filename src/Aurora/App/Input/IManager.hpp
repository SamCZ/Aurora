#pragma once
#include "Aurora/Core/Common.hpp"

#include <set>
#include <map>
#include <string>
#include <filesystem>
#include <optional>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <Aurora/App/ISystemWindow.hpp>

namespace Aurora::Input
{
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
        typedef std::map<Key_t, bool> UsedKeysInvertion_t;
        typedef std::map<Action_t, UsedKeysInvertion_t> Actions_t;
    protected:
        std::map<Category_t, Actions_t> m_Configurations = {};
    public:
        void LoadConfig_JSON(const nlohmann::json& jConfig);
        void LoadConfig_JSON(const std::filesystem::path& configFile);

    // Active category
    protected:
        std::string m_ActiveCategory = "loading";
    public: // Get
        [[nodiscard]] inline const std::string& GetActiveCategory() const noexcept { return m_ActiveCategory; }
    public: // Set
        virtual bool ActiveCategory(const std::string& category);

    // Cursor
    public: // Position
        /// Current position of cursor when cursor is not locked
        [[nodiscard]] virtual const std::optional<glm::dvec2>& CursorPosition()       const noexcept = 0;
        /// Position change between 2 frames with unlocked cursor
        [[nodiscard]] virtual                     glm::dvec2   CursorPositionChange() const noexcept = 0;
        /// Change of cursor position while the cursor is locked
        [[nodiscard]] virtual                     glm::dvec2   CursorChange()         const noexcept = 0;
    public: // Locking
        [[nodiscard]] virtual bool IsCursorLocked()        const = 0;
                      virtual void LockCursor(bool locked) const = 0;
    private:
        double m_CursorSensitivity = 1.0;
    public: // Get
        [[nodiscard]] inline double CursorSensitivity() const noexcept { return m_CursorSensitivity; }
    public: // Set
        inline void CursorSensitivity(double value) noexcept { m_CursorSensitivity = value; }

    public:
        [[nodiscard]] virtual bool   GetDigital         (const Action_t& action) = 0;
        [[nodiscard]] virtual bool   GetDigital_Pressed (const Action_t& action) = 0;
        [[nodiscard]] virtual bool   GetDigital_Released(const Action_t& action) = 0;
        [[nodiscard]] virtual double GetAnalog          (const Action_t& action) = 0;

    // Text input
    public:
    	virtual const std::vector<char8_t>& GetTypedChars() = 0;

    // Scroll
    public:
        virtual glm::dvec2 Scrolling() = 0;

    // Color
    public:
        /// Change color of peripherals.
        /// Currently implemented only using Steam Input:
        /// - Steam Controller - Grayscale only, changes brightness of the logo
        /// - Dualshock 4 - Changes color of the stripe
        /// In future may be extended to general peripheral color changes (like `LED ILLUMINATION SDK` from `Logitech G Developer Lab`)
        void SetPeripheralColor(glm::vec3 rgb);
    };
}
