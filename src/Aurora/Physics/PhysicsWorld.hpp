#pragma once

#include <ndWorld.h>

namespace Aurora
{
	class PhysicsWorld : public ndWorld
	{
	public:
		PhysicsWorld();
		~PhysicsWorld() override;
	};
}
