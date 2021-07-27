#include "MaterialLoader.hpp"
#include "AssetManager.hpp"
#include "../Graphics/Material.hpp"
#include "../Graphics/GraphicUtilities.hpp"
#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{
	MaterialLoader::MaterialLoader(const MaterialLoader::LoadType &loadType, Path path)
	: m_LoadType(loadType), m_Path(std::move(path)) { }

	MaterialLoader MaterialLoader::LoadShaderFolder(const Path &path)
	{
		return MaterialLoader(LoadType::ShaderFolder, path);
	}

	MaterialLoader MaterialLoader::LoadCompute(const Path &path)
	{
		return MaterialLoader(LoadType::Compute, path);
	}

	MaterialLoader MaterialLoader::Load(const Path &path)
	{
		return MaterialLoader(LoadType::MaterialFile, path);
	}

	MaterialLoader &MaterialLoader::AddMacro(const String &macro, const String &value)
	{
		m_Macros[macro] = value;
		return *this;
	}

	std::shared_ptr<Material> MaterialLoader::Finish()
	{
		std::shared_ptr<ShaderCollection> shaderCollection = nullptr;

		std::map<EShaderType, Shader_ptr> shaders;

		switch (m_LoadType) {
			case LoadType::MaterialFile: {
				return LoadMaterialFile(m_Path, m_Macros);
			}
			case LoadType::Compute: {
				//TODO: Load compute shader
				break;
			}
			case LoadType::ShaderFolder: {

				break;
			}
		}

		if(shaderCollection == nullptr) {
			AU_LOG_ERROR("Cannot load shader !", m_Path);
		}

		return std::make_shared<Material>(m_Path.filename().string(), m_Path, m_Macros);
	}

	void LoadCubeMap(std::shared_ptr<Material>& material, const String& name, nlohmann::json& value)
	{
		switch (value.size()) {
			case 6: {
				std::array<Path, 6> textures;

				int i = 0;
				for(auto& it : value) {
					auto path = it.get<String>();
					if(!ASM->FileExists(path)) {
						AU_LOG_ERROR("Cannot find texture ", path, " !");
					}

					textures[i++] = path;
				}

				auto texture = GraphicUtilities::CreateCubeMap(textures);

				if(texture == nullptr) {
					AU_LOG_ERROR("Cannot create cubemap ", name, " !");
				}

				material->SetTexture(name, texture);

				break;
			}
			case 1: {
				// TODO: Load equirectangular map or other
				break;
			}
			default:
				AU_LOG_WARNING("Cubemap ", name, "has unknown number of textures ", value.size(), " !");
		}
	}

	std::shared_ptr<Material> MaterialLoader::LoadMaterialFile(const Path& path, std::map<String, String> macros)
	{
		nlohmann::json json;

		if(!AuroraEngine::AssetManager->LoadJson(path, json)) {
			AU_LOG_ERROR("Cannot load ", path);
		}

		Shader_ptr shaderProgram = nullptr;

		if(json.find("shader_macros") != json.end()) {
			auto& shader_macros = json["shader_macros"];

			for(nlohmann::json::iterator it = shader_macros.begin(); it != shader_macros.end(); ++it) {
				macros[it.key()] = it.value().get<String>();
			}
		}

		if(json.find("shaderFolder") != json.end()) {
			shaderProgram = ASM->LoadShaderFolder(json["shaderFolder"].get<std::string>());
		} else {
			AU_LOG_ERROR("No shaders in ", path, " material file !");
		}

		auto material = std::make_shared<Material>(path.filename().string(), shaderProgram);

		if(json.find("variables") != json.end()) {
			auto& variables = json["variables"];

			for(auto& variable : variables) {
				const String& varName = variable["name"].get<String>();
				const String& varType = variable["type"].get<String>();
				auto& varValue = variable["value"];

				if(varType == "CubeMap") {
					LoadCubeMap(material, varName, varValue);
				}

				if(varType == "Texture") {
					auto texturePath = varValue.get<String>();
					auto texture = AuroraEngine::AssetManager->LoadTexture(texturePath, GraphicsFormat::SRGBA8_UNORM);

					if(texture == nullptr) {
						AU_LOG_ERROR("Cannot load texture ", texturePath, " !");
					}

					material->SetTexture(varName, texture);
				}

				// TODO: Complete other types
			}
		}

		return material;
	}
}