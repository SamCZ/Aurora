#pragma once

#include "../Base/PrimitiveComponent.hpp"
#include "../../Mesh/Mesh.hpp"
#include "../../../Graphics/Material.hpp"

namespace Aurora
{
	class MeshComponent : public PrimitiveComponent
	{
	protected:
		Mesh_ptr m_Mesh;
	public:
		inline MeshComponent() : PrimitiveComponent() {}
		~MeshComponent() override = default;
	public:
		inline void SetMesh(const Mesh_ptr& mesh)
		{
			m_Mesh = mesh;

			if(m_Mesh != nullptr) {
				m_Body.SetCollider(m_Mesh->m_Bounds.get());
				m_Body.Transform(GetTransformMatrix());
			} else {
				m_Body.SetCollider(nullptr);
			}
		}
		inline const Mesh_ptr& GetMesh() { return m_Mesh; }
	};
}