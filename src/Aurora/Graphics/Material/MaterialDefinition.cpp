#include "MaterialDefinition.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Material.hpp"

namespace Aurora
{
	uint64_t HashShaderMacros(const ShaderMacros& macros)
	{
		std::stringstream ss;

		for(const auto& it : macros)
		{
			ss << it.first;
			ss << it.second;
		}

		return std::hash<String>()(ss.str());
	}

	std::ostream& operator<<(std::ostream &out, ShaderMacros const& macros)
	{
		for(const auto& it : macros)
		{
			out << "#define " << it.first << " " << it.second << std::endl;
		}

		return out;
	}

	///////////////////////////////////// PassShaderDef /////////////////////////////////////

	MaterialPassDef::MaterialPassDef(ShaderProgramDesc shaderProgramDesc, MaterialPassState passState)
		: m_ShaderBaseDescription(std::move(shaderProgramDesc)), m_PassStates(std::move(passState)) {}

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
				AU_LOG_WARNING("Shader permutation error! ", macros);
				return nullptr;
			}

			m_ShaderPermutations[hash] = newShader;
			return newShader;
		}

		return it->second;
	}

	///////////////////////////////////// MaterialDefinition /////////////////////////////////////

	bool CanLoadBlock(const ShaderResourceDesc& block)
	{
		if(block.Name == "Instances")
			return false;

		if(block.Name == "BaseVSData")
			return false;

		// TODO: Find a better way to exclude global blocks

		return true;
	}

	MaterialDefinition::MaterialDefinition(const MaterialDefinitionDesc& desc)
		: m_Name(desc.Name), m_Path(desc.Filepath), m_PassDefs(desc.ShaderPasses.size())
	{
		size_t memorySize = 0;

		m_TextureVars = desc.Textures;

		std::vector<std::tuple<size_t, size_t, std::vector<float>>> defaultsToWrite;

		for(const auto& passIt : desc.ShaderPasses)
		{
			MaterialPassState passState;
			//TODO: read pass state from desc
			m_PassDefs[passIt.first] = MaterialPassDef(passIt.second, passState);

			m_PassUniformBlockMapping[passIt.first] = {};
			m_PassTextureMapping[passIt.first] = {};

			ShaderMacros macros; // TODO: Finish macros
			Shader_ptr shader = m_PassDefs[passIt.first].GetShader(macros);

			for(const ShaderResourceDesc& sampler : shader->GetResources(ShaderResourceType::Sampler))
			{
				String samplerName = sampler.Name;
				TTypeID samplerId = Hash_djb2(samplerName.c_str());

				const auto& it = m_TextureVars.find(samplerId);
				if(it == m_TextureVars.end())
				{
					MTextureVar textureVar;
					textureVar.Name = samplerName;
					textureVar.InShaderName = samplerName;
					textureVar.Texture = GEngine->GetResourceManager()->LoadTexture("Assets/Textures/blueprint.png", GraphicsFormat::SRGBA8_UNORM, {});
					textureVar.Sampler = Samplers::WrapWrapLinearLinear;
					m_TextureVars[samplerId] = textureVar;
				}

				m_PassTextureMapping[passIt.first].push_back(samplerId);
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
					if(skip) break;

					for(uint8 mappedBlockIndex : mappingIt.second)
					{
						MUniformBlock& mappedBlock = m_UniformBlocksDef[mappedBlockIndex];

						if(mappedBlock.Name == block.Name && mappedBlock.Size == block.Size && mappedBlock.Vars.size() == block.Variables.size())
						{
							m_PassUniformBlockMapping[passIt.first].emplace_back(mappedBlockIndex);
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

				std::cout << uniformBlock.Name << " - " << uniformBlock.NameID << std::endl;

				for(const auto& var : block.Variables)
				{
					TTypeID varId = Hash_djb2(var.Name.data());

					MUniformVar uniformVar;
					uniformVar.Name = var.Name;
					uniformVar.Size = var.Size;
					uniformVar.Offset = var.Offset;
					uniformVar.Connected = false;

					const auto& descVarIt = desc.Variables.find(varId);
					if(descVarIt != desc.Variables.end())
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

					std::cout << " - " << var.Name << " " << var.Size << std::endl;
				}

				m_PassUniformBlockMapping[passIt.first].emplace_back(currentBlockIndex);
				m_UniformBlocksDef.emplace_back(uniformBlock);

				memorySize += block.Size;
			}
		}

		m_BaseUniformData.resize(memorySize);

		// Write defaults
		for(auto& pair : defaultsToWrite)
		{
			size_t offset = std::get<0>(pair);
			size_t size = std::get<1>(pair);
			const std::vector<float>& values = std::get<2>(pair);

			std::memcpy(m_BaseUniformData.data() + offset, values.data(), size);
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
		auto mat = std::make_shared<Material>(this);
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
}