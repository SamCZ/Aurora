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

	bool ActorComponent::AttachToComponent(SceneComponent* InParent, const String& socket)
	{
		if (m_Parent == InParent)
		{
			return false;
		}

		DetachFromComponent();

		m_Parent = InParent;
		InParent->m_Components.push_back(this);

		m_Socket = socket;

		return true;
	}

	void ActorComponent::DetachFromComponent()
	{
		m_Socket = "";

		if (m_Parent)
		{
			VectorRemove<ActorComponent*>(m_Parent->m_Components, this);
			m_Parent = nullptr;
		}
	}
}