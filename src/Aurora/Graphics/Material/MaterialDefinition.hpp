#pragma once

#include <map>
#include <sstream>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Graphics/Base/ShaderBase.hpp"

#include "../PassType.hpp"

namespace Aurora
{
	class SMaterial;

	class MacroSet
	{
	private:
		String m_Name = "Unknown";
		ShaderMacros m_Macros;
	public:
		MacroSet() = default;
		explicit MacroSet(String name) : m_Name(std::move(name)), m_Macros() {}
		MacroSet(const MacroSet& other) = default;
		explicit MacroSet(ShaderMacros macros) : m_Macros(std::move(macros)) {}
		MacroSet(String name, ShaderMacros macros) : m_Name(std::move(name)), m_Macros(std::move(macros)) {}

		inline void Set(const String& key, const String& value)
		{
			m_Macros[key] = value;
		}

		inline void Remove(const String& key)
		{
			m_Macros.erase(key);
		}

		inline void Clear()
		{
			m_Macros.clear();
		}

		void Merge(const MacroSet& other)
		{
			for(const auto& it : other.m_Macros)
			{
				if(!m_Macros.contains(it.first))
				{
					m_Macros[it.first] = it.second;
				}
				else
				{
					AU_LOG_WARNING("Mergin macro sets resolved in collision of macro: ", it.first);
				}
			}
		}

		[[nodiscard]] uint64_t Hash() const
		{
			std::stringstream ss;

			for(const auto& it : m_Macros)
			{
				ss << it.first;
				ss << it.second;
			}

			return std::hash<String>()(ss.str());
		}

		[[nodiscard]] const ShaderMacros& Get() const
		{
			return m_Macros;
		}

		 [[nodiscard]] const String& GetName() const { return m_Name; }
	};

	std::ostream& operator<< (std::ostream &out, MacroSet const &t)
	{
		for(const auto& it : t.Get())
		{
			out << "#define " << it.first << " " << it.second << std::endl;
		}

		return out;
	}

	class PassShaderDef
	{
	private:
		ShaderProgramDesc m_ShaderBaseDescription;
		std::map<PassType_t, Shader_ptr> m_ShaderPermutations;
	public:
		PassShaderDef() = default;
		explicit PassShaderDef(ShaderProgramDesc shaderProgramDesc);
		Shader_ptr GetShader(const MacroSet& macroSet);
	};

	struct MaterialDefinitionDesc
	{
		String Name;
		Path Filepath;
		std::map<PassType_t, ShaderProgramDesc> ShaderPasses;
		std::vector<MacroSet> MacroSets;
		//TODO: variables
	};

	/*
	 * Holds variable patterns, macro sets, shader permutations
	 */
	class MaterialDefinition
	{
	private:
		String m_Name;
		Path m_Path;
		std::map<uint8, PassShaderDef> m_PassShaders;
	public:
		explicit MaterialDefinition(const MaterialDefinitionDesc& desc);

		std::shared_ptr<SMaterial> CreateInstance();
	};
}
