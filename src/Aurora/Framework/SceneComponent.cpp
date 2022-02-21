#include "SceneComponent.hpp"
#include "Aurora/Core/Common.hpp"

namespace Aurora
{

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
}