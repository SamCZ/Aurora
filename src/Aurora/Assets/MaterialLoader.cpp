#include "MaterialLoader.hpp"
#include "AssetManager.hpp"
#include "../Graphics/Material.hpp"
#include "../Graphics/GraphicUtilities.hpp"

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

		std::map<SHADER_TYPE, RefCntAutoPtr<IShader>> shaders;

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
			AU_THROW_ERROR("Cannot load shader !" << m_Path);
		}

		return std::make_shared<Material>(m_Path.filename().string(), m_Path, m_Macros);
	}

	void LoadCubeMap(std::shared_ptr<Material>& material, const String& name, nlohmann::json& value)
	{
		switch (value.size()) {
			case 6: {
				std::array<RefCntAutoPtr<ITexture>, 6> textures;

				int i = 0;
				for(auto& it : value) {
					auto path = it.get<String>();
					auto texture = AuroraEngine::AssetManager->LoadTexture(path);

					if(texture == nullptr) {
						AU_THROW_ERROR("Cannot find texture " << path <<" !");
					}

					textures[i++] = texture;
				}

				auto texture = GraphicUtilities::CreateCubeMap(textures);

				if(texture == nullptr) {
					AU_THROW_ERROR("Cannot create cubemap " << name << " !");
				}

				material->SetTexture(name, texture);

				break;
			}
			case 1: {
				// TODO: Load equirectangular map or other
				break;
			}
			default:
				AU_THROW_ERROR("Cubemap " << name << "has unknown number of textures " << value.size() << " !");
		}
	}

	std::shared_ptr<Material> MaterialLoader::LoadMaterialFile(const Path& path, std::map<String, String> macros)
	{
		nlohmann::json json;

		if(!AuroraEngine::AssetManager->LoadJson(path, json)) {
			AU_THROW_ERROR("Cannot load " << path);
		}

		std::vector<ShaderResourceObject_ptr> shaderCollection;

		if(json.find("shader_macros") != json.end()) {
			auto& shader_macros = json["shader_macros"];

			for(nlohmann::json::iterator it = shader_macros.begin(); it != shader_macros.end(); ++it) {
				macros[it.key()] = it.value().get<String>();
			}
		}

		if(json.find("shaders") != json.end()) {
			auto& shaders = json["shaders"];
			for(nlohmann::json::iterator it = shaders.begin(); it != shaders.end(); ++it) {
				const String& shader_type = it.key();
				const Path shader_path = it.value().get<String>();

				if(shader_type == "vertex") {
					shaderCollection.push_back(AuroraEngine::AssetManager->LoadShaderResource(shader_path, SHADER_SOURCE_LANGUAGE_GLSL, SHADER_TYPE_VERTEX));
				} else if(shader_type == "pixel") {
					shaderCollection.push_back(AuroraEngine::AssetManager->LoadShaderResource(shader_path, SHADER_SOURCE_LANGUAGE_GLSL, SHADER_TYPE_PIXEL));
				} else {
					AU_THROW_ERROR("Unknown shader type " << shader_type << " for material " << path);
				}
			}
		} else {
			AU_THROW_ERROR("No shaders in " << path << " material file !");
		}

		auto material = std::make_shared<Material>(path.filename().string(), shaderCollection, macros);

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
					auto texture = AuroraEngine::AssetManager->LoadTexture(texturePath);

					if(texture == nullptr) {
						AU_THROW_ERROR("Cannot load texture " << texturePath << " !");
					}

					material->SetTexture(varName, texture);
				}

				// TODO: Complete other types
			}
		}

		return material;
	}
}