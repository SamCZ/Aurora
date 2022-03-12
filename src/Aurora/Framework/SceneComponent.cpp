#include "SceneComponent.hpp"
#include "Aurora/Core/Common.hpp"
#include "Actor.hpp"

namespace Aurora
{
	SceneComponent::SceneComponent() : m_Parent(nullptr)
	{

	}

	Matrix4 SceneComponent::GetTransformationMatrix() const
	{
		// TODO: Cache matrix and update it only when transform changes!

		if(m_Parent)
		{
			return m_Parent->GetTransformationMatrix() * m_Transform.GetTransform();
		}

		return m_Transform.GetTransform();
	}

	bool SceneComponent::AttachToComponent(SceneComponent *InParent)
	{
		if (m_Parent == InParent)
		{
			return false;
		}

		DetachFromComponent();

		m_Parent = InParent;
		InParent->m_Components.push_back(this);

		return true;
	}

	void SceneComponent::DetachFromComponent()
	{
		if (m_Parent)
		{
			VectorRemove<SceneComponent*>(m_Parent->m_Components, this);
			m_Parent = nullptr;
		}
	}

	bool SceneComponent::IsParentActive() const
	{
		if(!HasParent())
		{
			return m_Owner->IsActive();
		}

		return m_Owner->IsActive() && m_Parent->IsActive();
	}
}