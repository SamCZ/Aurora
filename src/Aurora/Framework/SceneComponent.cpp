#include "SceneComponent.hpp"
#include "Aurora/Core/Common.hpp"
#include "Actor.hpp"

namespace Aurora
{
	SceneComponent::SceneComponent() : ActorComponent()
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
}