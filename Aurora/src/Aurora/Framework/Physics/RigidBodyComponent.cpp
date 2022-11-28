#include "RigidBodyComponent.hpp"
#include "../SceneComponent.hpp"

namespace Aurora
{
	Transform& RigidBodyComponent::GetWorldTransform()
	{
		return GetParent()->GetTransform();
	}
}