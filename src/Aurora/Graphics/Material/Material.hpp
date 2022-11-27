#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/Object.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Delegate.hpp"

#include "Aurora/Tools/robin_hood.h"

#include "Aurora/Graphics/PassType.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"

namespace Aurora
{
	class MaterialDefinition;

	[[nodiscard]] AU_API uint64_t HashShaderMacros(const ShaderMacros& macros);
	AU_API std::ostream& operator<<(std::ostream &out, ShaderMacros const& macros);

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
		StrHashID InShaderNameID;
		String Widget; // TODO: Change to enum
		std::vector<float> Numbers;

		[[nodiscard]] inline size_t MemorySize() const
		{
			return Numbers.size() * sizeof(float);
		}
	};

	struct MVarBase
	{
		String Name;
		bool HasEnableMacro;
		String MacroName;
	};

	struct MTextureVar : MVarBase
	{
		String InShaderName;
		Texture_ptr Texture;
		Sampler_ptr Sampler;
	};

	struct MUniformVar : MVarBase
	{
		size_t Size;
		size_t Offset;

		String ConnectedName;
		String Widget;
		bool Connected;
	};

	struct MUniformBlock
	{
		String Name;
		StrHashID NameID;
		size_t Size;
		size_t Offset;

		robin_hood::unordered_map<StrHashID, MUniformVar> Vars;

		MUniformVar* FindVar(StrHashID id)
		{
			const auto& it = Vars.find(id);

			if(it == Vars.end())
				return nullptr;

			return &it->second;
		}
	};

	struct MaterialPassState
	{
		FRasterState RasterState;
		FDepthStencilState DepthStencilState;
		FBlendState BlendState;

		MaterialPassState() : RasterState(), DepthStencilState(), BlendState() {}
	};

	typedef uint64_t SortID;

	enum class RenderSortType : uint8
	{
		Sky = 0,
		Opaque,
		Transparent,
		Translucent,
		Overlay,
		Count
	};

	static const char* RenderSortType_Strings[] = { "Sky", "Opaque", "Translucent", "Transparent" };

	enum MaterialFlags : uint8_t
	{
		MF_NONE = 0,
		MF_INSTANCED = 1 << 0,
		MF_TRANSFORM = 1 << 1,

	};

	static constexpr uint8_t SortTypeCount = (uint8) RenderSortType::Count;

	AU_CLASS(Material)
	{
	protected:
		MaterialDefinition* m_MatDef;

		std::vector<uint8> m_UniformData;
		robin_hood::unordered_map<StrHashID, MTextureVar> m_TextureVars;

		ShaderMacros m_Macros; // TODO: Finish macros

		robin_hood::unordered_map<PassType_t, MaterialPassState> m_PassStates;

		RenderSortType m_SortType = RenderSortType::Opaque;
		uint8_t m_Flags = MF_INSTANCED | MF_TRANSFORM;

		uint8_t m_StateCheck = 0;

	public:
		EventEmitter<PassType_t, DrawCallState&, class CameraComponent*, Material*> BeforeMaterialBegin;
	public:
		explicit Material(MaterialDefinition* matDef);
		Material(MaterialDefinition* matDef, bool instance);

		inline MaterialDefinition* GetMaterialDef() { return m_MatDef; }

		void SetSortType(RenderSortType sortType) { m_SortType = sortType; }
		[[nodiscard]] RenderSortType GetSortType() const { return m_SortType; }
		[[nodiscard]] uint8_t GetFlags() const { return m_Flags; }
		[[nodiscard]] bool HasFlag(uint8_t flag) const { return m_Flags & flag; }
		void SetFlags(uint8_t flags) { m_Flags = flags; }

		FRasterState& RasterState(PassType_t pass = 0);
		FDepthStencilState& DepthStencilState(PassType_t pass = 0);
		FBlendState& BlendState(PassType_t pass = 0);

		ShaderMacros& GetMacros() { return m_Macros; }
		[[nodiscard]] const ShaderMacros& GetMacros() const { return m_Macros; }
		void SetMacro(const String& key, const String& value) { m_Macros[key] = value; }

		void BeginPass(PassType_t pass, DrawCallState& state);
		void EndPass(PassType_t pass, DrawCallState& state);

		std::shared_ptr<Material> Clone();
		virtual void ReloadShader();

		void SetAlphaThresholdEnabled(bool alphaThreshold)
		{
			if (alphaThreshold)
			{
				m_Macros["USE_ALPHA_THRESHOLD"] = "1";
			}
			else if (m_Macros.contains("USE_ALPHA_THRESHOLD"))
			{
				m_Macros.erase("USE_ALPHA_THRESHOLD");
			}
		}

		[[nodiscard]] bool IsAlphaThresholdSet() const
		{
			return m_Macros.contains("USE_ALPHA_THRESHOLD");
		}

		uint8* GetBlockMemory(StrHashID id, size_t size);

		template<typename VarBlock>
		bool SetVarBlock(StrHashID id, VarBlock& block)
		{
			if(auto* blockPtr = reinterpret_cast<VarBlock*>(GetBlockMemory(id, sizeof(VarBlock))))
			{
				block = *blockPtr;
				return true;
			}

			return false;
		}

		template<typename VarBlock>
		VarBlock* GetVarBlock(StrHashID id)
		{
			return reinterpret_cast<VarBlock*>(GetBlockMemory(id, sizeof(VarBlock)));
		}

		//////// Variables ////////

		uint8* GetVariableMemory(StrHashID varId, size_t size);
		bool SetVariable(StrHashID varId, uint8* data, size_t size);

		template<typename VarType>
		bool SetVariable(StrHashID varId, VarType var)
		{
			auto* rawData = (uint8*)(&var);
			size_t size = sizeof(VarType);

			return SetVariable(varId, rawData, size);
		}

		template<typename VarType>
		bool GetVariable(StrHashID varId, VarType& outVar)
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
	public:
		MTextureVar* GetTextureVar(StrHashID varId);
	public:
		bool SetTexture(StrHashID varId, const Texture_ptr& texture);
		bool SetSampler(StrHashID varId, const Sampler_ptr& texture);

		Texture_ptr GetTexture(StrHashID varId);
		Sampler_ptr GetSampler(StrHashID varId);

		//////// Buffers ////////
		bool SetBuffer(StrHashID bufferId, const Buffer_ptr& buffer) { return false; } // TODO: Complete buffers
	};

	using matref = std::shared_ptr<Material>;
}
