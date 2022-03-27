#include "MaterialLoader.hpp"

#include <iostream>

#include "Aurora/Engine.hpp"
#include "Aurora/Logger/Logger.hpp"
#include "Aurora/Graphics/Material/MaterialDefinition.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

namespace Aurora
{

	bool MaterialLoader::ParseMaterialDefinitionJson(const nlohmann::json& json, const std::filesystem::path& path, MaterialDefinitionDesc& descOut)
	{
		if(!json.contains("name"))
		{
			return false;
		}

		descOut.Name = json["name"].get<String>();
		descOut.Filepath = path;

		if(!json.contains("passes"))
		{
			return false;
		}

		for(const auto& it : json["passes"])
		{
			auto passName = it["name"].get<String>();
			auto passShaders = it["shaders"];
			PassType_t passId = -1;

			for(const char* pass : PassTypesToString)
			{
				passId++;
				if(pass == passName)
				{
					break;
				}
			}

			std::map<EShaderType, Path> shaders;

			for(const auto& passShader : passShaders.items())
			{
				shaders[ShaderType_FromString(passShader.key())] = passShader.value().get<String>();
			}

			ShaderProgramDesc shaderProgramDesc = GEngine->GetResourceManager()->CreateShaderProgramDesc(descOut.Name + ":" + passName, shaders);

			if(!passId)
			{
				// TODO: Throw warning
			}

			descOut.ShaderPasses[passId] = shaderProgramDesc;
		}

		for(const auto& it : json["variables"])
		{
			auto name = it["name"].get<String>();
			auto typeName = it["type"].get<String>();
			auto in_shader_name = it["in_shader_name"].get<String>();
			auto value = it["value"];

			if(typeName == "Texture2D")
			{
				Sampler_ptr sampler = Samplers::WrapWrapLinearLinear;
				if(it.contains("Sampler"))
				{
					// TODO: Loading sampler definition
				}

				TTypeID nameId = Hash_djb2(name.c_str());

				MTextureVar textureVar;
				textureVar.Name = name;
				textureVar.InShaderName = in_shader_name;
				textureVar.Texture = GEngine->GetResourceManager()->LoadTexture(value.get<String>());
				textureVar.Sampler = sampler;

				if(descOut.Textures.contains(nameId))
				{
					AU_LOG_WARNING("Variable ", name, " with shader name ", in_shader_name, " already exists !");
					continue;
				}

				descOut.Textures[nameId] = textureVar;
			}
			else if(typeName == "Vector")
			{
				if(value.is_array())
				{
					MNumericValDesc numericValDesc;
					numericValDesc.Name = name;
					numericValDesc.InShaderName = in_shader_name;
					numericValDesc.InShaderNameID = Hash_djb2(in_shader_name.c_str());
					numericValDesc.Numbers = value.get<std::vector<float>>();

					if(descOut.Variables.contains(numericValDesc.InShaderNameID))
					{
						AU_LOG_WARNING("Variable ", name, " with shader name ", in_shader_name, " already exists !");
						continue;
					}

					descOut.Variables[numericValDesc.InShaderNameID] = numericValDesc;
				}
				else
				{
					AU_LOG_WARNING("Material system only accepts array is input even with one value !");
				}
			}
			else
			{
				AU_LOG_WARNING("Type " , typeName, " is not supported in materials!");
			}
		}

		return true;
	}
}