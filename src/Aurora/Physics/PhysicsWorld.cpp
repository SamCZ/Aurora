#include "PhysicsWorld.hpp"
#include "ndContactCallback.hpp"
#ifdef NEWTON
namespace Aurora
{
	PhysicsWorld::PhysicsWorld() : ndWorld()
	{
		ClearCache();
		SetContactNotify(new ndContactCallback());
	}

	PhysicsWorld::~PhysicsWorld() = default;
}
#endif