#include "MaterialDefinition.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "SMaterial.hpp"

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
			Shader_ptr newShader = GetEngine()->GetRenderDevice()->CreateShaderProgram(programDesc);

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

	MaterialDefinition::MaterialDefinition(const MaterialDefinitionDesc& desc)
		: m_Name(desc.Name), m_Path(desc.Filepath), m_PassDefs(desc.ShaderPasses.size())
	{
		size_t memorySize = 0;

		// TODO: merge same blocks together from different passes

		for(const auto& passIt : desc.ShaderPasses)
		{
			MaterialPassState passState;
			//TODO: read pass state from desc
			m_PassDefs[passIt.first] = MaterialPassDef(passIt.second, passState);

			ShaderMacros macros;
			Shader_ptr shader = m_PassDefs[passIt.first].GetShader(macros);

			for(const ShaderResourceDesc& block : shader->GetResources(ShaderResourceType::ConstantBuffer))
			{
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
					MUniformVar uniformVar;
					uniformVar.Name = var.Name;
					uniformVar.Size = var.Size;
					uniformVar.Offset = var.Offset;

					uniformBlock.Vars[Hash_djb2(var.Name.data())] = uniformVar;

					std::cout << " - " << var.Name << " " << var.Size << std::endl;
				}

				m_PassUniformBlockMapping[passIt.first].emplace_back(currentBlockIndex);
				m_UniformBlocksDef.emplace_back(uniformBlock);

				memorySize += block.Size;
			}
		}

		m_BaseUniformData.resize(memorySize);

		// TODO: Init default vars from desc
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

	std::shared_ptr<SMaterial> MaterialDefinition::CreateInstance(const MaterialOverrides &overrides)
	{
		auto mat = std::make_shared<SMaterial>(this);
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