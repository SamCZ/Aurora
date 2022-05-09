#include "PhysicsWorld.hpp"
#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/Physics/RigidBodyComponent.hpp"
#include "Aurora/Framework/Physics/ColliderComponent.hpp"

#include "Aurora/Graphics/DShape.hpp"

#include "AABBTree.hpp"

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
		ComponentView<ColliderComponent> colliderComponents = m_Scene->GetComponents<ColliderComponent>();
		AABBTree<ColliderComponent> aabbTree(colliderComponents.size());

		for (ColliderComponent* collider : colliderComponents)
		{
			AABB bounds = collider->GetAABB();
			bounds.SetOffset(collider->GetParent()->GetWorldPosition());
			aabbTree.InsertObject(collider, bounds);
		}

		for (const auto& node : aabbTree.GetNodes())
		{
			if (node.ParentNodeIndex == AABB_NULL_NODE || !node.IsLeaf())
				continue;

			DShapes::Box(node.Bounds, Color::green(), true, 1.5f, 0, false);
		}

		ComponentView<RigidBodyComponent> bodyComponents = m_Scene->GetComponents<RigidBodyComponent>();
		for (RigidBodyComponent* rigidBodyComponent : bodyComponents)
		{
			SceneComponent* parent = rigidBodyComponent->GetParent() != nullptr ? rigidBodyComponent->GetParent() : rigidBodyComponent->GetOwner()->GetRootComponent();
			std::vector<BoxColliderComponent*> colliders;
			parent->GetComponentsOfType(colliders);
			BoxColliderComponent* collider = colliders[0];

			Transform& transform = rigidBodyComponent->GetOwner()->GetRootComponent()->GetTransform();

			transform.AddLocation(0, -timeStep * 10.0f, 0);

			AABB bounds = collider->GetAABB();
			bounds.SetOffset(collider->GetParent()->GetWorldPosition());
			aabbTree.UpdateObject(collider, bounds);

			bool overlaps = false;

			for (auto overlap : aabbTree.QueryOverlaps(collider, bounds))
			{
				AABB boundsOther = overlap->GetAABB();
				boundsOther.SetOffset(overlap->GetParent()->GetWorldPosition());

				BBCollisionResult res = bounds.CollideWithOther(boundsOther);

				/*for (int i = 0; i < 3; ++i)
				{
					Vector3 dist;
					dist[i] = res.AxesDistances[i];
					transform.SetLocation(transform.GetLocation() + dist);
				}*/

				AABB intersection = bounds.Intersection(boundsOther);

				AU_LOG_INFO("Overlap ", parent->GetOwner()->GetName(), " with ", overlap->GetOwner()->GetName());
				AU_LOG_INFO("Overlap ", glm::to_string(collider->GetParent()->GetWorldPosition()));
				AU_LOG_INFO("Overlap ", glm::to_string(overlap->GetParent()->GetWorldPosition()));
				AU_LOG_INFO("Overlap ");
				AU_LOG_INFO("Overlap ", bounds);
				AU_LOG_INFO("Overlap ", boundsOther);
				AU_LOG_INFO("Overlap ", intersection);
				AU_LOG_INFO("Overlap ", glm::to_string(res.Distance));
				AU_LOG_INFO("Overlap ", glm::to_string(res.CollidingAxesV));

				//AU_LOG_FATAL("yo");

				transform.SetLocation(transform.GetLocation() + Vector3(0, intersection.GetHeight(), 0));
				overlaps = true;
				break;
				//transform.AddLocation(0, +timeStep * 10.0f, 0);
			}

			if (transform.GetLocation().y < 0.0 && !overlaps)
			{
				transform.SetLocation(transform.GetLocation() * Vector3(1, 0.0, 1));

				bounds = collider->GetAABB();
				bounds.SetOffset(collider->GetParent()->GetWorldPosition());
				aabbTree.UpdateObject(collider, bounds);
			}
		}
	}

	PhysicsWorld::~PhysicsWorld() = default;
}