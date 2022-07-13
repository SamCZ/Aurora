#include "MaterialDefinition.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Material.hpp"

namespace Aurora
{
	///////////////////////////////////// PassShaderDef /////////////////////////////////////

	MaterialPassDef::MaterialPassDef(ShaderProgramDesc shaderProgramDesc, MaterialPassState passState)
		: m_ShaderBaseDescription(std::move(shaderProgramDesc)), m_PassStates(passState) {}

	Shader_ptr MaterialPassDef::GetShader(const ShaderMacros& macros)
	{
		uint64_t hash = HashShaderMacros(macros);

		const auto& it = m_ShaderPermutations.find(hash);

		if(it == m_ShaderPermutations.end())
		{
			ShaderProgramDesc programDesc = m_ShaderBaseDescription;
			programDesc.AddShaderMacros(macros);
			Shader_ptr newShader = GEngine->GetRenderDevice()->CreateShaderProgram(programDesc);

			if(newShader == nullptr)
			{
				AU_LOG_WARNING("Shader permutation error! ");
				return nullptr;
			}

			m_ShaderPermutations[hash] = newShader;
			return newShader;
		}

		return it->second;
	}

	void MaterialPassDef::ReloadShader()
	{
		GEngine->GetResourceManager()->LoadShaderProgramSources(m_ShaderBaseDescription);
		m_ShaderPermutations.clear();
	}

	///////////////////////////////////// MaterialDefinition /////////////////////////////////////

	bool CanLoadBlock(const ShaderResourceDesc& block)
	{
		if(block.Name == "Instances")
			return false;

		if(block.Name == "BaseVSData")
			return false;

		if (block.Name.rfind("GLOB_", 0) == 0)
			return false;

		// TODO: Find a better way to exclude global blocks

		return true;
	}

	MaterialDefinition::MaterialDefinition(const MaterialDefinitionDesc& desc)
		: Material(this), m_Name(desc.Name), m_Path(desc.Filepath), m_PassDefs(desc.ShaderPasses.size())
	{
		size_t memorySize = 0;

		m_TextureVars = desc.Textures;

		ShaderMacros macros; // TODO: Fix this, looks like hack to me to enable all textures macros at startup, idk
		for (const auto& [TypeID, var] : m_TextureVars)
		{
			if (var.HasEnableMacro)
				macros[var.MacroName] = "1";
		}

		macros["USE_ALPHA_THRESHOLD"] = "1";

		std::vector<std::tuple<size_t, size_t, std::vector<float>>> defaultsToWrite;

		for(auto& [passType, passDesc] : desc.ShaderPasses)
		{
			MaterialPassState passState;
			//TODO: read pass state from desc
			m_PassDefs[passType] = MaterialPassDef(passDesc, passState);

			m_PassUniformBlockMapping[passType] = {};
			m_PassTextureMapping[passType] = {};

			Shader_ptr shader = m_PassDefs[passType].GetShader(macros);

			for(const ShaderResourceDesc& sampler : shader->GetResources(ShaderResourceType::Sampler))
			{
				String samplerName = sampler.Name;

				if(samplerName[0] == 'g' && samplerName[1] == '_')
					continue;

				TTypeID samplerId = Hash_djb2(samplerName.c_str());

				const auto& it = m_TextureVars.find(samplerId);
				if(it == m_TextureVars.end())
				{
					MTextureVar textureVar;
					textureVar.Name = samplerName;
					textureVar.HasEnableMacro = false;
					textureVar.MacroName = "";
					textureVar.InShaderName = samplerName;
					textureVar.Texture = nullptr;
					textureVar.Sampler = Samplers::WrapWrapLinearLinear;
					m_TextureVars[samplerId] = textureVar;
				}

				m_PassTextureMapping[passType].push_back(samplerId);
			}

			for(const ShaderResourceDesc& block : shader->GetResources(ShaderResourceType::ConstantBuffer))
			{
				if(!CanLoadBlock(block))
				{
					continue;
				}

				bool skip = false;

				// Check if same block already exists
				for(const auto& mappingIt : m_PassUniformBlockMapping)
				{
					if (skip) break;

					for (uint8 mappedBlockIndex : mappingIt.second)
					{
						MUniformBlock& mappedBlock = m_UniformBlocksDef[mappedBlockIndex];

						if (mappedBlock.Name == block.Name && mappedBlock.Size == block.Size && mappedBlock.Vars.size() == block.Variables.size())
						{
							m_PassUniformBlockMapping[passType].emplace_back(mappedBlockIndex);
							skip = true;
							AU_LOG_INFO("Found already cached block in material!");
							break;
						}
					}
				}

				if(skip) continue;

				MUniformBlock uniformBlock;
				uniformBlock.Name = block.Name;
				uniformBlock.NameID = Hash_djb2(block.Name.c_str());
				uniformBlock.Size = block.Size;
				uniformBlock.Offset = memorySize;

				size_t currentBlockIndex = m_UniformBlocksDef.size();

				for (const auto& var : block.Variables)
				{
					TTypeID varId = Hash_djb2(var.Name.data());

					MUniformVar uniformVar;
					uniformVar.Name = var.Name;
					uniformVar.Size = var.Size;
					uniformVar.Offset = var.Offset;
					uniformVar.Connected = false;

					const auto& descVarIt = desc.Variables.find(varId);
					if (descVarIt != desc.Variables.end())
					{
						const MNumericValDesc& valDesc = descVarIt->second;

						if(valDesc.MemorySize() == var.Size)
						{
							uniformVar.Connected = true;
							uniformVar.ConnectedName = valDesc.Name;
							uniformVar.Widget = valDesc.Widget;
							defaultsToWrite.emplace_back(uniformBlock.Offset + var.Offset, var.Size, valDesc.Numbers);
						}
						else
						{
							AU_LOG_WARNING("Connected variable ", valDesc.Name, " has wrong size ! You've set ", valDesc.MemorySize(), " but it needs ", var.Size, " !");
						}
					}

					uniformBlock.Vars[varId] = uniformVar;
				}

				m_PassUniformBlockMapping[passType].push_back((uint8)currentBlockIndex);
				m_UniformBlocksDef.emplace_back(uniformBlock);

				memorySize += block.Size;
			}
		}

		m_UniformData.resize(memorySize);

		// Write defaults
		for(auto& pair : defaultsToWrite)
		{
			size_t offset = std::get<0>(pair);
			size_t size = std::get<1>(pair);
			const std::vector<float>& values = std::get<2>(pair);

			std::memcpy(m_UniformData.data() + offset, values.data(), size);
		}

		for (auto& [TypeID, var] : m_TextureVars)
		{
			if (var.HasEnableMacro)
			 m_Macros[var.MacroName] = var.Texture != nullptr ? "1" : "0";

			if (!var.Texture)
				var.Texture = GEngine->GetResourceManager()->LoadTexture("Assets/Textures/blueprint.png");
		}
	}

	MUniformBlock* MaterialDefinition::FindUniformBlock(TTypeID id)
	{
		for(MUniformBlock& block : m_UniformBlocksDef)
		{
			if(block.NameID == id)
			{
				return &block;
			}
		}

		return nullptr;
	}

	MUniformVar* MaterialDefinition::FindUniformVar(TTypeID id, MUniformBlock** blockOut)
	{
		for(MUniformBlock& block : m_UniformBlocksDef)
		{
			MUniformVar* var = block.FindVar(id);

			if(var != nullptr)
			{
				if(blockOut != nullptr)
				{
					*blockOut = &block;
				}

				return var;
			}
		}

		return nullptr;
	}

	std::shared_ptr<Material> MaterialDefinition::CreateInstance(const MaterialOverrides &overrides)
	{
		auto mat = std::make_shared<Material>(this, true);
		AddRef(mat);
		return mat;
	}

	MaterialPassDef* MaterialDefinition::GetPassDefinition(uint8 pass)
	{
		const auto& it = m_PassDefs.find(pass);

		if(it == m_PassDefs.end())
		{
			return nullptr;
		}

		return &it->second;
	}

	Shader_ptr MaterialDefinition::GetShader(uint8 pass, const ShaderMacros &macroSet)
	{
		const auto& it = m_PassDefs.find(pass);

		if(it == m_PassDefs.end())
		{
			return nullptr;
		}

		return it->second.GetShader(macroSet);
	}

	void MaterialDefinition::ReloadShader()
	{
		for (auto& [type, def]: m_PassDefs)
		{
			def.ReloadShader();
		}
	}
}