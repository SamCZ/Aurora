#include "ActorComponent.hpp"
#include "SceneComponent.hpp"
#include "Actor.hpp"

namespace Aurora
{
	bool ActorComponent::IsParentActive() const
	{
		if(!HasParent())
		{
			return m_Owner->IsActive();
		}

		return m_Owner->IsActive() && m_Parent->IsActive();
	}

	bool ActorComponent::AttachToComponent(SceneComponent *InParent)
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

	void ActorComponent::DetachFromComponent()
	{
		if (m_Parent)
		{
			VectorRemove<ActorComponent*>(m_Parent->m_Components, this);
			m_Parent = nullptr;
		}
	}
}