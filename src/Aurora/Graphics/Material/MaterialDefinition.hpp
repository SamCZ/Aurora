#pragma once

#include <map>
#include <sstream>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Hash.hpp"
#include "Aurora/Graphics/Base/ShaderBase.hpp"
#include "Aurora/Tools/robin_hood.h"

#include "../PassType.hpp"

namespace Aurora
{
	class SMaterial;

	class MMacro
	{
	private:
		String Name;
	};

	class MMSwitchMacro
	{

	};

	struct MUniformVar
	{
		String Name;
		size_t Size;
		size_t Offset;
	};

	struct MUniformBlock
	{
		String Name;
		TTypeID NameID;
		size_t Size;
		size_t Offset;

		robin_hood::unordered_map<TTypeID, MUniformVar> Vars;

		MUniformVar* FindVar(TTypeID id)
		{
			const auto& it = Vars.find(id);

			if(it == Vars.end())
				return nullptr;

			return &it->second;
		}
	};

	[[nodiscard]] AU_API uint64_t HashShaderMacros(const ShaderMacros& macros);
	AU_API std::ostream& operator<<(std::ostream &out, ShaderMacros const& macros);

	class AU_API PassShaderDef
	{
	private:
		ShaderProgramDesc m_ShaderBaseDescription;
		std::map<PassType_t, Shader_ptr> m_ShaderPermutations;
	public:
		PassShaderDef() = default;
		explicit PassShaderDef(ShaderProgramDesc shaderProgramDesc);
		Shader_ptr GetShader(const ShaderMacros& macroSet);
	};

	struct MaterialDefinitionDesc
	{
		String Name;
		Path Filepath;
		std::map<PassType_t, ShaderProgramDesc> ShaderPasses;
		std::vector<ShaderMacros> MacroSets;
		//TODO: variables
	};

	struct MaterialOverrides
	{

	};

	/*
	 * Holds variable patterns, macro sets, shader permutations
	 */
	class AU_API MaterialDefinition
	{
		friend class SMaterial;
	private:
		String m_Name;
		Path m_Path;
		std::map<uint8, PassShaderDef> m_PassShaders;

		std::vector<MUniformBlock> m_UniformBlocksDef;
		robin_hood::unordered_map<uint8, std::vector<uint8>> m_PassUniformBlockMapping;

		std::vector<uint8> m_BaseUniformData;
	public:
		explicit MaterialDefinition(const MaterialDefinitionDesc& desc);

		std::shared_ptr<SMaterial> CreateInstance(const MaterialOverrides& overrides = {});
	private:
		MUniformBlock* FindUniformBlock(TTypeID id);
		MUniformVar* FindUniformVar(TTypeID id, MUniformBlock** blockOut = nullptr);
	};
}
