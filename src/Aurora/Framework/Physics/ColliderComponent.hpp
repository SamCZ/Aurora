#pragma once

#include "../ActorComponent.hpp"
#include "Aurora/Physics/AABB.hpp"

namespace Aurora
{
	class ColliderComponent : public ActorComponent
	{
	protected:
		AABB m_Bounds;
		Vector3 m_Origin;
	public:
		CLASS_OBJ(ColliderComponent, ActorComponent);

		ColliderComponent() : m_Bounds(), m_Origin() {}

		[[nodiscard]] const AABB& GetAABB() const { return m_Bounds; }
	};

	class BoxColliderComponent : public ColliderComponent
	{
	private:
		Vector3 m_Size;
	public:
		CLASS_OBJ(BoxColliderComponent, ColliderComponent);

		BoxColliderComponent() : ColliderComponent(), m_Size(1, 1, 1)
		{
			SetSize(1, 1, 1);
		}

		BoxColliderComponent(const Vector3& size) : ColliderComponent()
		{
			SetSize(size);
		}

		BoxColliderComponent(float x, float y, float z) : ColliderComponent()
		{
			SetSize(x, y, z);
		}

		inline void SetSize(const Vector3& size)
		{
			m_Bounds = AABB::FromExtent(m_Origin, size / 2.0f);
		}

		inline void SetSize(float x, float y, float z)
		{
			SetSize({x, y, z});
		}

	};
}
