#pragma once

#include <memory>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace Aurora
{
	struct MaterialDefinitionDesc;
	class MaterialDefinition;
	class Material;

	class MaterialLoader
	{
	public:
		static bool ParseMaterialDefinitionJson(const nlohmann::json& json, const std::filesystem::path& path, MaterialDefinitionDesc& descOut);
	};
}
