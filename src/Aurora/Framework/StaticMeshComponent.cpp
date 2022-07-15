#include "StaticMeshComponent.hpp"

namespace Aurora
{

	StaticMeshComponent::StaticMeshComponent() = default;

	void StaticMeshComponent::SetMesh(const Mesh_ptr& mesh)
	{
		if(!mesh)
		{
			m_Mesh = nullptr;
			return;
		}

		if(!mesh->HasType(GetSupportedMeshType()))
		{
			return;
		}

		m_Mesh = StaticMesh::Cast(mesh);
		m_MaterialSlots = m_Mesh->MaterialSlots;
	}
}