#pragma once

#include <map>
#include <sstream>

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Hash.hpp"
#include "Aurora/Tools/robin_hood.h"

#include "Aurora/Graphics/Base/ShaderBase.hpp"
#include "Aurora/Graphics/Base/Texture.hpp"
#include "Aurora/Graphics/Base/Sampler.hpp"
#include "Aurora/Graphics/Base/RasterState.hpp"
#include "Aurora/Graphics/Base/BlendState.hpp"
#include "Aurora/Graphics/Base/FDepthStencilState.hpp"

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

	struct MNumericValDesc
	{
		String Name;
		String InShaderName;
		TTypeID InShaderNameID;
		String Widget; // TODO: Change to enum
		std::vector<float> Numbers;

		[[nodiscard]] inline size_t MemorySize() const
		{
			return Numbers.size() * sizeof(float);
		}
	};

	struct MTextureVar
	{
		String Name;
		String InShaderName;
		Texture_ptr Texture;
		Sampler_ptr Sampler;
	};

	struct MNumericVal
	{
		String Name;
		TTypeID NameID;
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

	struct MaterialPassState
	{
		FRasterState RasterState;
		FDepthStencilState DepthStencilState;
		FBlendState BlendState;

		MaterialPassState() : RasterState(), DepthStencilState(), BlendState() {}
	};

	class AU_API MaterialPassDef
	{
	private:
		ShaderProgramDesc m_ShaderBaseDescription;
		robin_hood::unordered_map<PassType_t, Shader_ptr> m_ShaderPermutations;

		MaterialPassState m_PassStates;
	public:
		MaterialPassDef() = default;
		MaterialPassDef(ShaderProgramDesc shaderProgramDesc, MaterialPassState passState);
		Shader_ptr GetShader(const ShaderMacros& macroSet);

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
	class AU_API MaterialDefinition
	{
		friend class SMaterial;
	private:
		String m_Name;
		Path m_Path;

		std::vector<std::shared_ptr<SMaterial>> m_MaterialRefs;

		robin_hood::unordered_map<PassType_t, MaterialPassDef> m_PassDefs;

		std::vector<MUniformBlock> m_UniformBlocksDef;
		robin_hood::unordered_map<PassType_t, std::vector<uint8>> m_PassUniformBlockMapping;
		robin_hood::unordered_map<PassType_t, std::vector<TTypeID>> m_PassTextureMapping;

		std::vector<uint8> m_BaseUniformData;
		robin_hood::unordered_map<TTypeID, MTextureVar> m_TextureVars;
	public:
		explicit MaterialDefinition(const MaterialDefinitionDesc& desc);

		MaterialPassDef* GetPassDefinition(uint8 pass);
		Shader_ptr GetShader(uint8 pass, const ShaderMacros& macroSet);

		std::shared_ptr<SMaterial> CreateInstance(const MaterialOverrides& overrides = {});

		[[nodiscard]] inline const String& GetName() const { return m_Name; }
		[[nodiscard]] inline const Path& GetPath() const { return m_Path; }
	private:
		MUniformBlock* FindUniformBlock(TTypeID id);
		MUniformVar* FindUniformVar(TTypeID id, MUniformBlock** blockOut = nullptr);
		void AddRef(const std::shared_ptr<SMaterial>& mat) { m_MaterialRefs.emplace_back(mat); }
	};
}
