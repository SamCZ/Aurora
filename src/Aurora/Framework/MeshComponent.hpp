#pragma once

#include "SceneComponent.hpp"
#include "Mesh/Mesh.hpp"

namespace Aurora
{
	class AU_API MeshComponent : public SceneComponent
	{
	protected:
		MaterialSet m_MaterialSlots;
		bool m_IgnoreFrustumChecks = false;
	public:
		CLASS_OBJ(MeshComponent, SceneComponent);

		MeshComponent() = default;
		~MeshComponent() override = default;

		[[nodiscard]] virtual Mesh_ptr GetMesh() const = 0;
		virtual void SetMesh(const Mesh_ptr& mesh) = 0;
		[[nodiscard]] virtual bool HasMesh() const = 0;

		[[nodiscard]] virtual TTypeID GetSupportedMeshType() const = 0;

		void SetIgnoreFrustumChecks(bool ignoreFrustum = true) { m_IgnoreFrustumChecks = ignoreFrustum; }
		[[nodiscard]] bool IsIgnoringFrustumChecks() const { return m_IgnoreFrustumChecks; }

		void SetMaterial(int slot, const matref& material)
		{
			au_assert(slot < m_MaterialSlots.size());

			m_MaterialSlots[slot].Material = material;
		}

		[[nodiscard]] size_t GetNumMaterialSlots() const { return m_MaterialSlots.size(); }
		MaterialSlot& GetMaterialSlot(int slot)
		{
			au_assert(slot < m_MaterialSlots.size());
			return m_MaterialSlots[slot];
		}

		[[nodiscard]] const MaterialSlot& GetMaterialSlot(int slot) const
		{
			au_assert(slot < m_MaterialSlots.size());
			return m_MaterialSlots.find(slot)->second;
		}

		MaterialSet& GetMaterialSet() { return m_MaterialSlots; }
	};
}