#pragma once

#include <map>
#include <sstream>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Hash.hpp"
#include "Aurora/Tools/robin_hood.h"

#include "Material.hpp"

#include "../PassType.hpp"

namespace Aurora
{
	class AU_API MaterialPassDef
	{
	private:
		ShaderProgramDesc m_ShaderBaseDescription;
		std::unordered_map<PassType_t, Shader_ptr> m_ShaderPermutations;

		MaterialPassState m_PassStates;
	public:
		MaterialPassDef() = default;
		MaterialPassDef(ShaderProgramDesc shaderProgramDesc, MaterialPassState passState);
		Shader_ptr GetShader(const ShaderMacros& macroSet);
		void ReloadShader();

		inline MaterialPassState& GetMaterialPassState() { return m_PassStates; }
		[[nodiscard]] inline const MaterialPassState& GetMaterialPassState() const { return m_PassStates; }
	};

	struct MaterialDefinitionDesc
	{
		String Name;
		Path Filepath;
		robin_hood::unordered_map<PassType_t, ShaderProgramDesc> ShaderPasses;
		std::vector<ShaderMacros> MacroSets;
		//TODO: variables
		robin_hood::unordered_map<TTypeID, MTextureVar> Textures;
		robin_hood::unordered_map<TTypeID, MNumericValDesc> Variables;
	};

	struct MaterialOverrides
	{

	};

	/*
	 * Holds variable patterns, macro sets, shader permutations
	 */
	AU_CLASS(MaterialDefinition) : public Material
	{
		friend class Material;
	private:
		String m_Name;
		Path m_Path;

		std::vector<std::weak_ptr<Material>> m_MaterialRefs;

		robin_hood::unordered_map<PassType_t, MaterialPassDef> m_PassDefs;

		std::vector<MUniformBlock> m_UniformBlocksDef;
		robin_hood::unordered_map<PassType_t, std::vector<uint8>> m_PassUniformBlockMapping;
		robin_hood::unordered_map<PassType_t, std::vector<TTypeID>> m_PassTextureMapping;
	public:
		explicit MaterialDefinition(const MaterialDefinitionDesc& desc);

		MaterialPassDef* GetPassDefinition(uint8 pass);
		Shader_ptr GetShader(uint8 pass, const ShaderMacros& macroSet);
		void ReloadShader() override;

		std::shared_ptr<Material> CreateInstance(const MaterialOverrides& overrides = {});

		[[nodiscard]] inline const String& GetName() const { return m_Name; }
		[[nodiscard]] inline const Path& GetPath() const { return m_Path; }

		[[nodiscard]] const std::vector<MUniformBlock>& GetUniformBlocks() const { return m_UniformBlocksDef; }
		[[nodiscard]] const robin_hood::unordered_map<TTypeID, MTextureVar>& GetTextureVars() const { return m_TextureVars; }
		[[nodiscard]] const std::vector<std::weak_ptr<Material>>& GetMaterialRefs() const { return m_MaterialRefs; }
	private:
		MUniformBlock* FindUniformBlock(TTypeID id);
		MUniformVar* FindUniformVar(TTypeID id, MUniformBlock** blockOut = nullptr);
		void AddRef(const std::shared_ptr<Material>& mat) { m_MaterialRefs.emplace_back(mat); }
	};
}
