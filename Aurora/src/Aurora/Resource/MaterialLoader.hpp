#pragma once

#include <memory>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "Aurora/Core/Library.hpp"

namespace Aurora
{
	struct MaterialDefinitionDesc;
	class MaterialDefinition;
	class Material;

	class AU_API MaterialLoader
	{
	public:
		static bool ParseMaterialDefinitionJson(const nlohmann::json& json, const std::filesystem::path& path, MaterialDefinitionDesc& descOut);
	};
}
