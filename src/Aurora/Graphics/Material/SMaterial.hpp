#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/PassType.hpp"

#include "MaterialDefinition.hpp"

namespace Aurora
{
	class SMaterial
	{
	private:
		MaterialDefinition* m_MatDef;
		std::vector<uint8> m_UniformData;	// All of the uniíforms blocks from passes packed in here
		ShaderMacros m_Macros;

		robin_hood::unordered_map<PassType_t, MaterialPassState> m_PassStates;
	public:
		explicit SMaterial(MaterialDefinition* matDef);
		~SMaterial();

		void BeginPass(PassType_t pass, DrawCallState& state);
		void EndPass(PassType_t pass, DrawCallState& state);

		FRasterState& RasterState(PassType_t pass = 0);
		FDepthStencilState& DepthStencilState(PassType_t pass = 0);
		FBlendState& BlendState(PassType_t pass = 0);

		std::shared_ptr<SMaterial> Clone();
		//////// Blocks ////////
	private:
		uint8* GetBlockMemory(TTypeID id, size_t size);
	public:
		template<typename VarBlock>
		bool SetVarBlock(TTypeID id, VarBlock& block)
		{
			if(auto* blockPtr = reinterpret_cast<VarBlock*>(GetBlockMemory(id, sizeof(VarBlock))))
			{
				block = *blockPtr;
				return true;
			}

			return false;
		}

		template<typename VarBlock>
		VarBlock* GetVarBlock(TTypeID id)
		{
			return reinterpret_cast<VarBlock*>(GetBlockMemory(id, sizeof(VarBlock)));
		}

		//////// Variables ////////

		uint8* GetVariableMemory(TTypeID varId, size_t size);
		bool SetVariable(TTypeID varId, uint8* data, size_t size);

		template<typename VarType>
		bool SetVariable(TTypeID varId, VarType var)
		{
			auto* rawData = (uint8*)(&var);
			size_t size = sizeof(VarType);

			return SetVariable(varId, rawData, size);
		}

		template<typename VarType>
		bool GetVariable(TTypeID varId, VarType& outVar)
		{
			size_t size = sizeof(VarType);
			uint8* memory = GetVariableMemory(varId, size);

			if(!memory)
			{
				return false;
			}

			outVar = *reinterpret_cast<VarType*>(memory);
			return true;
		}
	};
}
