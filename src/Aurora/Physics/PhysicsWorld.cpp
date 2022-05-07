#include "PhysicsWorld.hpp"
#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/Physics/RigidBodyComponent.hpp"

namespace Aurora
{
	PhysicsWorld::PhysicsWorld(Scene* scene) : m_Scene(scene), m_Accumulator(0), m_Time(0)
	{

	}

	void PhysicsWorld::Update(double frameTime)
	{
		if (frameTime > 0.25)
			frameTime = 0.25;

		const double timeStep = 1.0 / 60.0;
		m_Accumulator += frameTime;

		while (m_Accumulator >= timeStep)
		{
			RunPhysics(m_Time, timeStep);
			m_Time += timeStep;
			m_Accumulator -= timeStep;
		}
	}

	void PhysicsWorld::RunPhysics(double time, double timeStep)
	{
		ComponentView<RigidBodyComponent> components = m_Scene->GetComponents<RigidBodyComponent>();


	}

	PhysicsWorld::~PhysicsWorld() = default;
}