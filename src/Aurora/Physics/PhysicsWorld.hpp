#pragma once
#ifdef NEWTON
#include <dNewton/ndWorld.h>

namespace Aurora
{
	class PhysicsWorld : public ndWorld
	{
	public:
		PhysicsWorld();
		~PhysicsWorld() override;
	};
}
#endif