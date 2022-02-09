#include "SMaterial.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/RenderManager.hpp"

namespace Aurora
{
	SMaterial::SMaterial(MaterialDefinition* matDef)
		: m_MatDef(matDef), m_UniformData(matDef->m_BaseUniformData)
	{

	}

	SMaterial::~SMaterial() = default;

	///////////////////////////////////// RENDER PASS /////////////////////////////////////
#pragma region RenderPass

	void SMaterial::BeginPass(uint8 pass, DrawCallState& drawState)
	{
		IRenderDevice* renderDevice = GetEngine()->GetRenderDevice();

		Shader_ptr shader = m_MatDef->GetShader(pass, m_Macros);

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
			uint8* data = GetEngine()->GetRenderManager()->GetUniformBufferCache().GetOrMap(block.Size, cacheIndex);
			std::memcpy(data, m_UniformData.data() + block.Offset, block.Size);
			GetEngine()->GetRenderManager()->GetUniformBufferCache().Unmap(cacheIndex);
			drawState.BindUniformBuffer(block.Name, cacheIndex.Buffer, cacheIndex.Offset, cacheIndex.Size);
		}

		renderDevice->BindShaderResources(drawState);

		//TODO: Set render states
	}

	void SMaterial::EndPass(uint8 pass, DrawCallState& state)
	{
		GetEngine()->GetRenderManager()->GetUniformBufferCache().Reset();
	}

#pragma endregion RenderPass

	///////////////////////////////////// BLOCKS /////////////////////////////////////

	uint8* SMaterial::GetBlockMemory(TTypeID id, size_t size)
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

	uint8 *SMaterial::GetVariableMemory(TTypeID varId, size_t size)
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

	bool SMaterial::SetVariable(TTypeID varId, uint8 *data, size_t size)
	{
		uint8* varMemoryStart = GetVariableMemory(varId, size);

		if(!varMemoryStart)
		{
			return false;
		}

		std::memcpy(varMemoryStart, data, size);
		return true;
	}
}