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
			if (not m_Socket.empty())
			{
				int32_t socketIndex = m_Parent->GetSocketIndex(m_Socket);

				if (socketIndex >= 0)
				{
					return m_Parent->GetTransformationMatrix() * m_Parent->GetSocketTransform(socketIndex) * m_Transform.GetTransform();
				}
			}

			return m_Parent->GetTransformationMatrix() * m_Transform.GetTransform();
		}

		return m_Transform.GetTransform();
	}
}