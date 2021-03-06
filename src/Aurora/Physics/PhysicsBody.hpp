#pragma once

#include "Aurora/Core/Common.hpp"
#include "BoundingBox.hpp"

namespace Aurora
{
	class PhysicsBody
	{
	private:
		Bound* m_Collider;
		BoundingBox m_Transformed;
	public:
		inline PhysicsBody() : m_Collider(nullptr)
		{

		}

		inline void SetCollider(Bound* boundingBox)
		{
			m_Collider = boundingBox;
		}

		bool Collide(const PhysicsBody& other)
		{
			if(m_Collider == nullptr) {
				return false;
			}

			//return m_Collider->IntersectsWith(other.m_Collider);
			return false;
		}

		void Transform(const Matrix4& matrix)
		{
			if(HasCollider()) {
				BoundingBox bounds = *(BoundingBox*)GetBounds();
				bounds *= matrix;
				m_Transformed = bounds;
			}
		}

		inline bool HasCollider()
		{
			return m_Collider != nullptr;
		}

		inline Bound* GetBounds()
		{
			return m_Collider;
		}

		inline BoundingBox& GetTransformedBounds()
		{
			return m_Transformed;
		}
	};
}