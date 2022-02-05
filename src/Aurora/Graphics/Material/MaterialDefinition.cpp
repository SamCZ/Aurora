#include "MaterialDefinition.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"

namespace Aurora
{
	///////////////////////////////////// PassShaderDef /////////////////////////////////////

	PassShaderDef::PassShaderDef(ShaderProgramDesc shaderProgramDesc) : m_ShaderBaseDescription(std::move(shaderProgramDesc)) {}

	Shader_ptr PassShaderDef::GetShader(const MacroSet& macroSet)
	{
		uint64_t hash = macroSet.Hash();

		const auto& it = m_ShaderPermutations.find(hash);

		if(it == m_ShaderPermutations.end())
		{
			ShaderProgramDesc programDesc = m_ShaderBaseDescription;
			programDesc.AddShaderMacros(macroSet.Get());
			Shader_ptr newShader = GetEngine()->GetRenderDevice()->CreateShaderProgram(programDesc);

			if(newShader == nullptr)
			{
				AU_LOG_WARNING("Shader permutation error! ", macroSet);
				return nullptr;
			}

			m_ShaderPermutations[hash] = newShader;
		}

		return it->second;
	}

	///////////////////////////////////// MaterialDefinition /////////////////////////////////////

	MaterialDefinition::MaterialDefinition(const MaterialDefinitionDesc& desc)
		: m_Name(desc.Name), m_Path(desc.Filepath)
	{
		for(const auto& passIt : desc.ShaderPasses)
		{
			m_PassShaders[passIt.first] = PassShaderDef(passIt.second);
		}
	}
}