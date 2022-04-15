#include "IManager.hpp"

#include <regex>
#include <iostream>

#include <fstream>
#include <Aurora/Logger/Logger.hpp>

namespace Aurora::Input
{
    static std::regex regex_binding_name = std::regex("^[a-z0-9_]+$"); // NOLINT(cert-err58-cpp)

    bool IManager::ActiveCategory(const std::string& category) // Setter
    {
    	if(m_ActiveCategory == category) {
			return false;
    	}

        m_ActiveCategory = category;
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
}
