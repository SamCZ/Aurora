#pragma once

#include "MeshComponent.hpp"

namespace Aurora
{
	class AU_API StaticMeshComponent : public MeshComponent
	{
	private:
		StaticMesh_ptr m_Mesh;
	public:
		CLASS_OBJ(StaticMeshComponent, MeshComponent);

		StaticMeshComponent();
		~StaticMeshComponent() override = default;

		[[nodiscard]] Mesh_ptr GetMesh() const override { return m_Mesh; }
		[[nodiscard]] const StaticMesh_ptr& GetStaticMesh() const { return m_Mesh; }
		[[nodiscard]] bool HasMesh() const override { return m_Mesh != nullptr; }

		void SetMesh(const Mesh_ptr& mesh) override;

		[[nodiscard]] TTypeID GetSupportedMeshType() const override { return StaticMesh::TypeID(); }
	};
}