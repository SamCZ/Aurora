#pragma once

#include "MaterialBase.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/PassType.hpp"

#include <entt/core/enum.hpp>
#include <entt/core/type_traits.hpp>

namespace Aurora
{
	class ResourceManager;

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

	class AU_API Material : public MaterialBase
	{
	public:
		CLASS_OBJ(Material, MaterialBase)
	private:
		std::map<EPassType, Shader_ptr> m_Shaders;
		RenderSortType m_SortType = RenderSortType::Opaque;
		uint8_t m_Flags = MF_INSTANCED | MF_TRANSFORM;
	public:
		~Material() override = default;

		virtual void OnShaderReload(ResourceManager* rsm) {}
		void Load() {}

		virtual void BeginPass(DrawCallState& drawState, EPassType passType) const;
		virtual void EndPass(DrawCallState& drawState, EPassType passType) {}

		void SetSortType(RenderSortType sortType) { m_SortType = sortType; }
		[[nodiscard]] RenderSortType GetSortType() const { return m_SortType; }
		[[nodiscard]] uint8_t GetFlags() const { return m_Flags; }
		[[nodiscard]] bool HasFlag(uint8_t flag) const { return m_Flags & flag; }
	protected:
		void SetShader(EPassType passType, const Shader_ptr& shader)
		{
			m_Shaders[passType] = shader;
		}

		[[nodiscard]] Shader_ptr GetShader(EPassType passType) const
		{
			auto it = m_Shaders.find(passType);
			au_assert(it != m_Shaders.end());
			return it->second;
		}

		void SetFlags(uint8_t flags) { m_Flags = flags; }
	};

	using matref = std::shared_ptr<Material>;
}
