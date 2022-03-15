#pragma once

#include "MeshComponent.hpp"

namespace Aurora
{
	class StaticMeshComponent : public MeshComponent
	{
	private:
		StaticMesh_ptr m_Mesh;
	public:
		CLASS_OBJ(StaticMeshComponent, MeshComponent);

		[[nodiscard]] Mesh_ptr GetMesh() const override { return m_Mesh; }
		[[nodiscard]] const StaticMesh_ptr& GetStaticMesh() const { return m_Mesh; }
		[[nodiscard]] bool HasMesh() const override { return m_Mesh != nullptr; }

		void SetMesh(const Mesh_ptr& mesh) override
		{
			if(!mesh)
				return;

			if(!mesh->HasType(GetSupportedMeshType()))
			{
				return;
			}

			m_Mesh = StaticMesh::Cast(mesh);
			m_MaterialSlots = m_Mesh->MaterialSlots;
		}

		[[nodiscard]] TTypeID GetSupportedMeshType() const override { return StaticMesh::TypeID(); }
	};
}