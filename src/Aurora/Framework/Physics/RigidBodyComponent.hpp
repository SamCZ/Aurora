#pragma once

#include "../ActorComponent.hpp"
#include "Aurora/Physics/TransformUtil.hpp"

namespace Aurora
{
	class RigidBodyComponent : public ActorComponent
	{
	private:
		bool m_IsInSleep;
		bool m_CanSleep;
		bool m_HasGravity;
		bool m_IsKinematic;

		Vector3 m_Velocity;
		Vector3 m_AngularVelocity;
		Vector3 m_Acceleration;
	public:
		CLASS_OBJ(RigidBodyComponent, ActorComponent);

		RigidBodyComponent() : ActorComponent(), m_Velocity(0), m_AngularVelocity(0), m_Acceleration(0)
		{

		}

		inline void SetVelocity(const Vector3& velocity) { m_Velocity = velocity; }
		inline void AddVelocity(const Vector3& velocity) { m_Velocity += velocity; }
		[[nodiscard]] inline const Vector3& GetVelocity() const { return m_Velocity; }

		inline void SetAcceleration(const Vector3& acceleration) { m_Acceleration = acceleration; }
		inline void AddAcceleration(const Vector3& acceleration) { m_Acceleration += acceleration; }
		[[nodiscard]] inline const Vector3& GetAcceleration() const { return m_Acceleration; }

		[[nodiscard]] bool HasGravity() const
		{
			return m_HasGravity;
		}

		void SetHasGravity(bool mHasGravity)
		{
			m_HasGravity = mHasGravity;
		}

		[[nodiscard]] bool IsKinematic() const
		{
			return m_IsKinematic;
		}

		void SetMIsKinematic(bool mIsKinematic)
		{
			m_IsKinematic = mIsKinematic;
		}

		void PredictIntegratedTransform(phScalar timeStep, Transform& predictedTransform);

		Transform& GetWorldTransform();
	};
}
