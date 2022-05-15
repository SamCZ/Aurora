#include "ColliderComponent.hpp"

#include "../SceneComponent.hpp"

namespace Aurora
{
	AABB ColliderComponent::GetTransformedAABB() const
	{
		AABB bounds = GetAABB();
		bounds.SetOffset(GetParent()->GetWorldPosition());
		return bounds;
	}
}