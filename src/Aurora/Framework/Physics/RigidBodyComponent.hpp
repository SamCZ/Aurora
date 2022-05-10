#pragma once

#include "../ActorComponent.hpp"

namespace Aurora
{
	class RigidBodyComponent : public ActorComponent
	{
	private:
		bool m_IsInSleep;
		bool m_CanSleep;
		bool m_HasGravity;
		bool m_IsKinematic;
	public:
		CLASS_OBJ(RigidBodyComponent, ActorComponent);

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
	};
}
