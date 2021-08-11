#pragma once

#include "Aurora/Core/Common.hpp"
#include <ndWorld.h>

namespace Aurora
{
	AU_CLASS(PhysicsWorld) : public ndWorld
	{
	public:
		PhysicsWorld();
		~PhysicsWorld() override;
	};
}
