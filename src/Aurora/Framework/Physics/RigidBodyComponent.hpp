#pragma once

#include "../ActorComponent.hpp"

namespace Aurora
{
	class RigidBodyComponent : public ActorComponent
	{
	private:
		bool m_IsInSleep;
		bool m_CanSleep;
	public:
		CLASS_OBJ(RigidBodyComponent, ActorComponent);
	};
}
