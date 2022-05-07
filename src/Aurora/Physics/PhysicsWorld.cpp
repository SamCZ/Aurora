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
		ComponentView<RigidBodyComponent> bodyComponents = m_Scene->GetComponents<RigidBodyComponent>();

		for (RigidBodyComponent* rigidBodyComponent : bodyComponents)
		{
			SceneComponent* parent = rigidBodyComponent->GetParent() != nullptr ? rigidBodyComponent->GetParent() : rigidBodyComponent->GetOwner()->GetRootComponent();

			Transform& transform = rigidBodyComponent->GetOwner()->GetRootComponent()->GetTransform();

			transform.AddLocation(0, -timeStep * 10.0f, 0);

			if (transform.GetLocation().y < 0.0)
			{
				transform.SetLocation(transform.GetLocation() * Vector3(1, 0.0, 1));
			}
		}
	}

	PhysicsWorld::~PhysicsWorld() = default;
}