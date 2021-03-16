#include "PhysicsSystem.hpp"
#include "Aurora/Physics/MotionIntegrators.hpp"
#include <GLFW/glfw3.h>

namespace Aurora
{
	PhysicsSystem::PhysicsSystem() : m_TerminalVelocity(57.0)
	{

	}

	PhysicsSystem::~PhysicsSystem()
	{
		for(auto* subSystem : m_SubCollisionSystems) {
			delete subSystem;
		}
	}

	std::vector<PhysicsBody*> FindBodies(Actor* actor)
	{
		std::vector<PhysicsBody*> bodies;

		for(auto* component : actor->FindComponentsOfType<SceneComponent>())
		{
			PhysicsBody& body = component->GetBody();

			if(body.HasCollider()) {
				bodies.push_back(&body);
			}
		}

		return bodies;
	}

	void Aurora::PhysicsSystem::Update(const std::vector<SceneComponent *> &components, double delta)
	{
		static Vector3D gravity = Vector3D(0, -30, 0); //-9.81
		static double object_mass = 1;

		std::map<Actor*, std::vector<PhysicsBody*>> cachedBodies;

		static auto GetBodies = [&cachedBodies](Actor* actor) -> std::vector<PhysicsBody*>*
		{
			std::vector<PhysicsBody*>* bodies;

			auto it = cachedBodies.find(actor);

			if(it != cachedBodies.end()) {
				bodies = &it->second;
			} else {
				cachedBodies[actor] = FindBodies(actor);
				bodies = &cachedBodies[actor];
			}

			return bodies;
		};

		int count = 0;
		auto start = glfwGetTime();

		size_t componentCount = components.size();

		for (int i = 0; i < componentCount; ++i) {
			auto* component = components[i];

			Actor* who = component->GetOwner();

			if(!component->IsSimulatingPhysics() || !component->IsRootComponent() || !component->IsActive() || !who->IsActive()) {
				continue;
			}

			//TODO: optimize this by caching this values
			auto currentBodies = GetBodies(who);

			Vector3D position = component->GetLocation();
			Vector3D velocity = component->GetVelocity();
			Vector3D acceleration = component->GetAcceleration();


			velocity += acceleration;
			velocity += object_mass * gravity * delta;

			ApplyTerminalVelocity(velocity);

			// Apply collisions

			for(auto* subSystem : m_SubCollisionSystems) {
				if(subSystem == nullptr) continue;

				if(!CollisionMatrix::CanCollide(component->GetLayer(), subSystem->Layer())) {
					continue;
				}

				for(auto* body : *currentBodies) {
					subSystem->Collide(who, component, body, position, velocity, acceleration, delta);
					count++;
				}
			}

			for (int j = 0; j < componentCount; ++j) {
				auto* component2 = components[j];
				Actor* target = component2->GetOwner();

				if(component == component2 || !component2->IsActive() || !target->IsActive()) {
					continue;
				}

				// TODO: Think about this, because idk if it is the right operation
				/*if(!component2->IsSimulatingPhysics()) {
					continue;
				}*/

				if(!CollisionMatrix::CanCollide(component->GetLayer(), component2->GetLayer())) {
					continue;
				}

				/*auto& currentBody = component->GetBody();
				auto& targetBody = component2->GetBody();

				if(currentBody.Collide(targetBody) == false) {
					continue;
				}*/

				count++;

				auto targetBodies = GetBodies(target);

				//std::cout << "current bodies: " << currentBodies->size() << ", target bodies: " << targetBodies->size() << std::endl;

				/*auto& currentColliders = currentBody.GetColliders();
				auto& targetColliders = targetBody.GetColliders();

				for(auto currentCollider : currentColliders) {
					for(auto targetCollider : targetColliders) {
						if(CanPerformCollision(currentCollider, targetCollider)) {
							continue;
						}

						PerformCollision(currentCollider, targetCollider, position, velocity);
					}
				}*/

				// TODO: Perform physics to modify position / velocity
			}

			// Apply motion integrators

			//MotionIntegrators::ForestRuth(position, velocity, acceleration, delta);

			position += velocity * delta;

			float friction = 0.8;
			velocity.x *= friction;
			velocity.z *= friction;

			component->SetLocation(position);
			component->SetVelocity(velocity);
			component->SetAcceleration(acceleration);

			//component->SetAcceleration(Vector3D(0, 0, 0));
		}

		auto end = glfwGetTime();
		auto time = (end - start) * 1000;

		//std::cout << "physics: " << components.size() << ", " << count << ", " << time << "ms" << std::endl;
	}

	void PhysicsSystem::ApplyTerminalVelocity(Vector3D& vec) const
	{
		vec = glm::clamp(vec, -m_TerminalVelocity, m_TerminalVelocity);
	}
}