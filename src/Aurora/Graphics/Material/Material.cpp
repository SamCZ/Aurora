#include "Material.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

#include "MaterialDefinition.hpp"

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

	Material::Material(MaterialDefinition *matDef) : m_MatDef(matDef)
	{

	}

	Material::Material(MaterialDefinition* matDef, bool instance)
		:  m_MatDef(matDef), m_UniformData(matDef->m_UniformData), m_Macros(matDef->m_Macros), m_PassStates(matDef->m_PassStates), m_TextureVars(matDef->m_TextureVars)
	{
		for(const auto& it : m_MatDef->m_PassDefs)
		{
			m_PassStates[it.first] = it.second.GetMaterialPassState();
		}
	}

	FRasterState& Material::RasterState(PassType_t pass)
	{
		return m_PassStates[pass].RasterState;
	}

	FDepthStencilState& Material::DepthStencilState(PassType_t pass)
	{
		return m_PassStates[pass].DepthStencilState;
	}

	FBlendState& Material::BlendState(PassType_t pass)
	{
		return m_PassStates[pass].BlendState;
	}

#pragma region RenderPass

	void Material::BeginPass(PassType_t pass, DrawCallState& drawState)
	{
		IRenderDevice* renderDevice = GEngine->GetRenderDevice();

		MaterialPassDef* passDef = m_MatDef->GetPassDefinition(pass);

		if(passDef == nullptr)
		{
			//TODO: Do something
			AU_LOG_WARNING("Pass ", pass, " not found for for ", m_MatDef->m_Name);
		}

		Shader_ptr shader = passDef->GetShader(m_Macros);

		if(shader == nullptr)
		{
			//TODO: Do something
			AU_LOG_WARNING("Shader for pass", pass, " is null !");
		}

		drawState.Shader = shader;
		renderDevice->SetShader(shader);

		// Set buffers

		for(uint8 uniformBlockIndex : m_MatDef->m_PassUniformBlockMapping[pass])
		{
			const MUniformBlock& block = m_MatDef->m_UniformBlocksDef[uniformBlockIndex];

			VBufferCacheIndex cacheIndex;
			uint8* data = GEngine->GetRenderManager()->GetUniformBufferCache().GetOrMap(block.Size, cacheIndex);
			std::memcpy(data, m_UniformData.data() + block.Offset, block.Size);
			GEngine->GetRenderManager()->GetUniformBufferCache().Unmap(cacheIndex);
			drawState.BindUniformBuffer(block.Name, cacheIndex.Buffer, cacheIndex.Offset, cacheIndex.Size);
		}

		// Set textures
		for(TTypeID texId : m_MatDef->m_PassTextureMapping[pass])
		{
			MTextureVar* textureVar = GetTextureVar(texId);

			if(textureVar->Texture == nullptr)
			{
				// TODO: Look at this and maybe fix this
				drawState.BindTexture(textureVar->InShaderName, GEngine->GetResourceManager()->LoadTexture("Assets/Textures/blueprint.png"));
				continue;
			}

			drawState.BindTexture(textureVar->InShaderName, textureVar->Texture);
			drawState.BindSampler(textureVar->InShaderName, textureVar->Sampler);
		}

		renderDevice->BindShaderResources(drawState);

		MaterialPassState& passState = m_PassStates[pass];
		renderDevice->SetRasterState(passState.RasterState);
		renderDevice->SetDepthStencilState(passState.DepthStencilState);
		// TODO: Set blend state


	}

	void Material::EndPass(PassType_t pass, DrawCallState& state)
	{
		(void)pass;
		(void)state;

		GEngine->GetRenderManager()->GetUniformBufferCache().Reset();
	}

	std::shared_ptr<Material> Material::Clone()
	{
		auto cloned = std::make_shared<Material>(m_MatDef, true);
		cloned->m_UniformData = m_UniformData;
		cloned->m_PassStates = m_PassStates;
		cloned->m_TextureVars = m_TextureVars;
		cloned->m_Macros = m_Macros;

		m_MatDef->AddRef(cloned);

		return cloned;
	}

#pragma endregion RenderPass

	uint8* Material::GetBlockMemory(TTypeID id, size_t size)
	{
		MUniformBlock* block = m_MatDef->FindUniformBlock(id);

		if(block == nullptr)
		{
			AU_LOG_WARNING("Requested block ", id, " not found !");
			return nullptr;
		}

		if(block->Size != size)
		{
			AU_LOG_WARNING("Requested block ", id, " has incorrect size, ActualBlockSize(", block->Size, ") vs RequestedBlockSize(", size, ")");
			return nullptr;
		}

		uint8* memoryStart = m_UniformData.data() + block->Offset;

		// TODO: do ifdef or some global var for validation
		if(true) // Validate memory
		{
			uint8* memoryEnd = memoryStart + size;
			uint8* lastMemoryAddress = &m_UniformData[m_UniformData.size() - 1] + 1;

			if(memoryEnd > lastMemoryAddress)
			{
				AU_LOG_WARNING("Memory out of bounds for block ", id, " !");
				return nullptr;
			}
		}

		return memoryStart;
	}

	uint8* Material::GetVariableMemory(TTypeID varId, size_t size)
	{
		MUniformBlock* block = nullptr;
		MUniformVar* var = m_MatDef->FindUniformVar(varId, &block);

		if(var == nullptr)
		{
			AU_LOG_WARNING("Requested variable ", varId, " not found !");
			return nullptr;
		}

		if(var->Size != size)
		{
			AU_LOG_WARNING("Requested variable ", varId, " has incorrect size, ActualVarSize(", var->Size, ") vs RequestedVarSize(", size, ")");
			return nullptr;
		}

		uint8* blockMemoryStart = m_UniformData.data() + block->Offset;
		uint8* varMemoryStart = blockMemoryStart + var->Offset;

		// TODO: do ifdef or some global var for validation
		if(true) // Validate memory
		{
			uint8* blockMemoryEnd = blockMemoryStart + block->Size;
			uint8* varMemoryEnd = varMemoryStart + var->Size;

			if(varMemoryEnd > blockMemoryEnd)
			{
				AU_LOG_WARNING("Memory out of bounds for variable ", varId, " !");
			}
		}

		return varMemoryStart;
	}

	bool Material::SetVariable(TTypeID varId, uint8 *data, size_t size)
	{
		uint8* varMemoryStart = GetVariableMemory(varId, size);

		if(!varMemoryStart)
		{
			return false;
		}

		std::memcpy(varMemoryStart, data, size);
		return true;
	}

	///////////////////////////////////// TEXTURES /////////////////////////////////////

	MTextureVar* Material::GetTextureVar(TTypeID varId)
	{
		const auto& it = m_TextureVars.find(varId);

		if(it == m_TextureVars.end())
		{
			const auto& it2 = m_MatDef->m_TextureVars.find(varId);

			if(it2 == m_MatDef->m_TextureVars.end())
			{
				AU_LOG_WARNING("Texture property ", varId, " not found !");
				return nullptr;
			}

			return &(m_TextureVars[varId] = it2->second);
		}

		return &it->second;
	}

	bool Material::SetTexture(TTypeID varId, const Texture_ptr& texture)
	{
		MTextureVar* var = GetTextureVar(varId);

		if(var == nullptr)
			return false;

		var->Texture = texture;

		if (var->HasEnableMacro)
			m_Macros[var->MacroName] = var->Texture != nullptr ? "1" : "0";

		return true;
	}

	bool Material::SetSampler(TTypeID varId, const Sampler_ptr& sampler)
	{
		MTextureVar* var = GetTextureVar(varId);

		if(var == nullptr)
			return false;

		var->Sampler = sampler;
		return true;
	}

	Texture_ptr Material::GetTexture(TTypeID varId)
	{
		MTextureVar* var = GetTextureVar(varId);

		if(var == nullptr)
			return nullptr;

		return var->Texture;
	}

	Sampler_ptr Material::GetSampler(TTypeID varId)
	{
		MTextureVar* var = GetTextureVar(varId);

		if(var == nullptr)
			return nullptr;

		return var->Sampler;
	}
}