#include "PhysicsWorld.hpp"
#include "ndContactCallback.hpp"

namespace Aurora
{

	PhysicsWorld::PhysicsWorld() : ndWorld()
	{
		ClearCache();
		SetContactNotify(new ndContactCallback());
	}

	PhysicsWorld::~PhysicsWorld()
	{

	}
}