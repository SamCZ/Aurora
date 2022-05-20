#pragma once

#include "../ActorComponent.hpp"
#include "../Transform.hpp"

namespace Aurora
{
	class RigidBodyComponent : public ActorComponent
	{
	private:
		bool m_IsInSleep;
		bool m_CanSleep;
		bool m_HasGravity;
		bool m_IsKinematic;

		float m_Mass;
		float m_Friction;

		Vector3 m_Velocity;
		Vector3 m_AngularVelocity;
		Vector3 m_Acceleration;
	public:
		bool CollidedSides[3];

		CLASS_OBJ(RigidBodyComponent, ActorComponent);

		RigidBodyComponent() :
			ActorComponent(),
			m_IsInSleep(false),
			m_CanSleep(true),
			m_HasGravity(true),
			m_IsKinematic(false),
			m_Mass(1),
			m_Friction(0),
			m_Velocity(0),
			m_AngularVelocity(0),
			m_Acceleration(0)
		{

		}

		inline void SetVelocity(const Vector3& velocity) { m_Velocity = velocity; }
		inline void AddVelocity(const Vector3& velocity) { m_Velocity += velocity; }
		[[nodiscard]] inline const Vector3& GetVelocity() const { return m_Velocity; }

		inline void SetAcceleration(const Vector3& acceleration) { m_Acceleration = acceleration; }
		inline void AddAcceleration(const Vector3& acceleration) { m_Acceleration += acceleration; }
		[[nodiscard]] inline const Vector3& GetAcceleration() const { return m_Acceleration; }

		inline void SetMass(float mass) { m_Mass = mass; }
		[[nodiscard]] inline float GetMass() const { return m_Mass; }

		inline void SetFriction(float friction) { m_Friction = friction; }
		[[nodiscard]] inline float GetFriction() const { return m_Friction; }

		[[nodiscard]] bool HasGravity() const { return m_HasGravity; }
		void SetHasGravity(bool mHasGravity) { m_HasGravity = mHasGravity; }

		[[nodiscard]] bool IsKinematic() const { return m_IsKinematic; }
		void SetIsKinematic(bool mIsKinematic) { m_IsKinematic = mIsKinematic; }

		Transform& GetWorldTransform();
	};
}
