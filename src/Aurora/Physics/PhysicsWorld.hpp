#pragma once

#include "Aurora/Core/Library.hpp"
#include "Aurora/Core/Vector.hpp"

namespace Aurora
{
	class Scene;

	class AU_API PhysicsWorld
	{
	private:
		Scene* m_Scene;
		double m_Time;
		double m_Accumulator;
	public:
		PhysicsWorld(Scene* scene);
		~PhysicsWorld();

		void Update(double frameTime);
	private:
		void RunPhysics(double time, double timeStep);
	};
}