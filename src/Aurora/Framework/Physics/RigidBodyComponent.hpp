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

		phVector3 m_LinearVelocity;
		phVector3 m_AngularVelocity;
	public:
		CLASS_OBJ(RigidBodyComponent, ActorComponent);

		RigidBodyComponent() : ActorComponent(), m_LinearVelocity(0), m_AngularVelocity(0)
		{

		}

		inline void SetLinearVelocity(const phVector3& velocity) { m_LinearVelocity = velocity; }

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
