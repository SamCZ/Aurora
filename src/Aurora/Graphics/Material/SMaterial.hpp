#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/PassType.hpp"

#include "MaterialDefinition.hpp"

namespace Aurora
{
	typedef uint64_t SortID;

	enum class RenderSortType : uint8
	{
		Opaque = 0,
		Translucent,
		Transparent,
		Sky,
		Count
	};

	enum MaterialFlags : uint8_t
	{
		MF_NONE = 0,
		MF_INSTANCED = 1 << 0,
		MF_TRANSFORM = 1 << 1,

	};

	static constexpr uint8_t SortTypeCount = (uint8) RenderSortType::Count;

	AU_CLASS(SMaterial)
	{
	private:
		MaterialDefinition* m_MatDef;
		std::vector<uint8> m_UniformData;	// All of the uniforms blocks from passes packed in here
		ShaderMacros m_Macros; // TODO: Finish macros

		robin_hood::unordered_map<PassType_t, MaterialPassState> m_PassStates;
		robin_hood::unordered_map<TTypeID, MTextureVar> m_TextureVars;

		RenderSortType m_SortType = RenderSortType::Opaque;
		uint8_t m_Flags = MF_INSTANCED | MF_TRANSFORM;
	public:
		explicit SMaterial(MaterialDefinition* matDef);
		~SMaterial();

		void BeginPass(PassType_t pass, DrawCallState& state);
		void EndPass(PassType_t pass, DrawCallState& state);

		FRasterState& RasterState(PassType_t pass = 0);
		FDepthStencilState& DepthStencilState(PassType_t pass = 0);
		FBlendState& BlendState(PassType_t pass = 0);

		std::shared_ptr<SMaterial> Clone();

		void SetSortType(RenderSortType sortType) { m_SortType = sortType; }
		[[nodiscard]] RenderSortType GetSortType() const { return m_SortType; }
		[[nodiscard]] uint8_t GetFlags() const { return m_Flags; }
		[[nodiscard]] bool HasFlag(uint8_t flag) const { return m_Flags & flag; }
		void SetFlags(uint8_t flags) { m_Flags = flags; }
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

		//////// Textures ////////
	private:
		MTextureVar* GetTextureVar(TTypeID varId);
	public:
		bool SetTexture(TTypeID varId, const Texture_ptr& texture);
		bool SetSampler(TTypeID varId, const Sampler_ptr& texture);

		Texture_ptr GetTexture(TTypeID varId);
		Sampler_ptr GetSampler(TTypeID varId);

		//////// Buffers ////////
		bool SetBuffer(TTypeID bufferId, const Buffer_ptr& buffer) { return false; } // TODO: Complete buffers
	};

	using matref = std::shared_ptr<SMaterial>;
}
