#include "IManager.hpp"

#include <regex>
#include <iostream>

#include <fstream>
#include <Aurora/Logger/Logger.hpp>

namespace Aurora::Input
{
    static std::regex regex_binding_name = std::regex("^[a-z0-9_]+$"); // NOLINT(cert-err58-cpp)

    Binding_ptr IManager::Binding(std::set<std::string> categories, const std::string& action)
    {
        if(action.empty())
            throw std::runtime_error("No input name specified");
        bool anyButton = action == "*";
        if(!anyButton && !std::regex_match(action, regex_binding_name))
            throw std::runtime_error("Binding name is not valid");


        if(categories.empty())
            throw std::runtime_error("No category specified");
        bool anyCategory = categories.contains("*");
        if(anyCategory)
            categories.clear();

        bool active = anyCategory || categories.contains(m_ActiveCategory);

        Binding_ptr binding = Binding_ptr(new class Binding(categories, action, active));
        m_KnownBindings.emplace(binding);
        return binding;
    }

    bool IManager::ActiveCategory(const std::string& category) // Setter
    {
    	if(m_ActiveCategory == category) {
			return false;
    	}

        m_ActiveCategory = category;

        // Update `Active` of all bindings
        for(auto& binding : m_KnownBindings)
        {
            if(binding->m_Categories.empty())
                binding->m_Active = true;
            else
                binding->m_Active = binding->m_Categories.contains(m_ActiveCategory);

            if(!binding->m_Active)
            {
                binding->m_ValueCurrent = 0;
                binding->m_HeldTime = 0;
            }
        }

        return true;
    }

    void IManager::LoadConfig_JSON(const nlohmann::json& jConfig)
    {
        int version = jConfig.contains("version") ? jConfig["version"].get<int>() : 0;
        switch(version)
        {
            default:
                throw std::runtime_error("Unsupported config version");

            case 1:
            {
                m_Configurations.clear();
#ifdef DEBUG
				AU_LOG_INFO("Cleared input configurations, loading new");
#endif

                for (auto itCategory = jConfig.begin(); itCategory != jConfig.end(); ++itCategory)
                {
                    if(itCategory.key() == "version")
                        continue;

                    const auto& jCategory = itCategory.value();
                    if(!jCategory.is_object())
                        throw std::runtime_error("Unexpected category type");

                    auto& cCategory = m_Configurations[itCategory.key()];
#ifdef DEBUG
                    std::size_t actionCount = 0;
                    std::size_t inputSourceCount = 0;
#endif
                    for (auto itAction = jCategory.begin(); itAction != jCategory.end(); ++itAction)
                    {
                        const auto& jAction = itAction.value();
                        if(!jAction.is_array())
                            throw std::runtime_error("Unexpected action type");

                        auto& cAction = cCategory[itAction.key()];
#ifdef DEBUG
                        actionCount++;
#endif
                        for (auto& jValue : jAction)
                        {
                            const std::string& key = jValue.get<std::string>();
                            if(key.empty())
                            {
                                AU_LOG_ERROR("Empty key in an Action");
                                continue;
                            }
                            bool inverted = key[0] == '-';
                            cAction.emplace(inverted ? key.substr(1) : key, inverted);
#ifdef DEBUG
                            inputSourceCount++;
#endif
                        }
                    }
#ifdef DEBUG
					AU_LOG_INFO("\t", itCategory.key(), ": ", actionCount, " actions with ", inputSourceCount, " input sources total");
#endif
                }
                return;
            }
        }
    }

    void IManager::LoadConfig_JSON(const std::filesystem::path& configFile)
    {
        if(!std::filesystem::exists(configFile) || !std::filesystem::is_regular_file(configFile))
            throw std::runtime_error("File Not Found (or is not a file)");

        std::fstream in(configFile, std::ios_base::in);
        if(!in.good())
            throw std::runtime_error("File Read Problem");

        nlohmann::json j;
        in >> j;

        LoadConfig_JSON(j);
    }

    void IManager::CurrentInputType(InputType value) noexcept
    {
#ifdef AU_INPUT_GAMEPAD_VISUAL_FORCE
        if(!InputType_IsGamepad(value))
            value = ControllerType::Gamepad_ABXY;
#endif
#ifdef AU_INPUT_GAMEPAD_ABXY_FORCE
        if(value != ControllerType::KeyboardAndMouse && value != ControllerType::Gamepad_ABXY)
            value = ControllerType::Gamepad_ABXY;
#endif
#ifdef AU_INPUT_GAMEPAD_PICTOGRAM_FORCE
        if(value != ControllerType::KeyboardAndMouse && value != ControllerType::Gamepad_Pictogram)
            value = ControllerType::Gamepad_Pictogram;
#endif
        m_InputType = value;
		AU_LOG_INFO("Changed Current Input Type to ", to_string(value));
    }

    void IManager::LockedInputType(std::optional<InputType> value) noexcept
    {
        if(value.has_value())
        {
#ifdef AU_INPUT_GAMEPAD_VISUAL_FORCE
            if(value.value() == ControllerType::KeyboardAndMouse)
                value = ControllerType::Gamepad_ABXY;
#endif
#ifdef AU_INPUT_GAMEPAD_ABXY_FORCE
            if(value.value() != ControllerType::KeyboardAndMouse && value.value() != ControllerType::Gamepad_ABXY)
                value = ControllerType::Gamepad_ABXY;
#endif
#ifdef AU_INPUT_GAMEPAD_PICTOGRAM_FORCE
            if(value.value() != ControllerType::KeyboardAndMouse && value.value() != ControllerType::Gamepad_Pictogram)
                value = ControllerType::Gamepad_Pictogram;
#endif
        }

        m_LockedInputType = value;

#ifdef DEBUG
        if(value.has_value()) {
            AU_LOG_INFO("Changed Locked Input Type to ", to_string(value.value()));
        } else {
            AU_LOG_INFO("Cleared Locked Input Type");
        }
#endif
    }
}
