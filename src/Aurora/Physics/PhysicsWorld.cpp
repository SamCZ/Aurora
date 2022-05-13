#include "PhysicsWorld.hpp"

#include "Aurora/Core/Profiler.hpp"
#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/Physics/RigidBodyComponent.hpp"
#include "Aurora/Framework/Physics/ColliderComponent.hpp"

#include "Aurora/Graphics/DShape.hpp"

#include "AABBTree.hpp"
#include "AABBUtil.hpp"

namespace Aurora
{
	//AABBTree<ColliderComponent> aabbTree(1);
	AABBTreeRaw aabbTreeRaw(1);

	PhysicsWorld::PhysicsWorld(Scene* scene) : m_Scene(scene), m_Accumulator(0), m_Time(0), m_DebugRender(false), m_Gravity(0, -0.5f, 0), m_UpdateRate(1.0 / 120.0)
	{

	}

	void PhysicsWorld::Update(double frameTime)
	{
		CPU_DEBUG_SCOPE("PhysicsWorld");

		m_Accumulator += frameTime;

		while (m_Accumulator >= m_UpdateRate)
		{
			RunPhysics();
			m_Time += m_UpdateRate;
			m_Accumulator -= m_UpdateRate;
		}

		if (IsDebugRender())
		{
			for (const auto& node : aabbTreeRaw.GetNodes())
			{
				if (node.parentNodeIndex == AABB_NULL_NODE || !node.isLeaf())
					continue;

				if (node.isLeaf())
					DShapes::Box(node.aabb, Color::red(), true, 1.2f, 0, false);
				else
					DShapes::Box(node.aabb, Color::green(), true, 1.0f, 0, false);
			}
		}
	}

	void PhysicsWorld::RunPhysics()
	{
		ComponentView<ColliderComponent> colliderComponents = m_Scene->GetComponents<ColliderComponent>();
		aabbTreeRaw = AABBTreeRaw(colliderComponents.size() * 2);

		for (ColliderComponent* collider : colliderComponents)
		{
			AABB bounds = collider->GetAABB();
			bounds.SetOffset(collider->GetParent()->GetTransform().GetLocation());
			aabbTreeRaw.insertObject(collider, bounds);
		}

		ComponentView<RigidBodyComponent> bodyComponents = m_Scene->GetComponents<RigidBodyComponent>();
		for (RigidBodyComponent* rigidBodyComponent : bodyComponents)
		{
			if (rigidBodyComponent->IsKinematic())
				continue;

			rigidBodyComponent->GetOwner()->FixedStep();

			SceneComponent* parent = rigidBodyComponent->GetParent() != nullptr ? rigidBodyComponent->GetParent() : rigidBodyComponent->GetOwner()->GetRootComponent();
			std::vector<BoxColliderComponent*> colliders;
			parent->GetComponentsOfType(colliders);

			Transform& transform = rigidBodyComponent->GetOwner()->GetRootComponent()->GetTransform();

			Vector3 velocity = rigidBodyComponent->GetVelocity();

			if (true) // if gravity
			{
				velocity += m_Gravity;
			}

			velocity += rigidBodyComponent->GetAcceleration();

			if (glm::length2(velocity) == 0) continue;

			transform.SetLocation(transform.GetLocation() + velocity * (float)m_UpdateRate);

			for (BoxColliderComponent* collider : colliders)
			{
				//Vector3 predictedLocation = transform.GetLocation() + velocity * (float)m_UpdateRate;

				// Predict bounding box
				AABB predictedBounds = collider->GetAABB();
				predictedBounds.SetOffset(transform.GetLocation());
				aabbTreeRaw.updateObject(collider, predictedBounds);

				for (auto collisionObject : aabbTreeRaw.queryOverlaps(collider, predictedBounds))
				{
					AABB boundsOther = collisionObject->GetAABB();
					boundsOther.SetOffset(collisionObject->GetParent()->GetWorldPosition());

					if (predictedBounds.GetMin().y < boundsOther.GetMax().y)
					{
						//AU_LOG_INFO("diff ", (boundsOther.GetMax().y - predictedBounds.GetMin().y) );

						transform.AddLocation(0, boundsOther.GetMax().y - predictedBounds.GetMin().y, 0);
						velocity.y = 0;
						//velocity.y += 0.5f + boundsOther.GetMax().y - predictedBounds.GetMin().y;
					}

				}

				// Revert bounds back
				AABB bounds = collider->GetAABB();
				bounds.SetOffset(collider->GetParent()->GetWorldPosition());
				aabbTreeRaw.updateObject(collider, bounds);
			}

			rigidBodyComponent->SetVelocity(velocity);
			//transform.SetLocation(transform.GetLocation() + velocity * (float)m_UpdateRate);
		}
	}

	PhysicsWorld::~PhysicsWorld() = default;

	int32_t PhysicsWorld::RayCast(const Vector3& fromPos, const Vector3& toPos, std::vector<RayCastHitResult>& results) const
	{
		ComponentView<ColliderComponent> colliderComponents = m_Scene->GetComponents<ColliderComponent>();

		Vector3 direction = glm::normalize(toPos - fromPos);
		float maxDistance = glm::length(toPos - fromPos);

		int32_t count = 0;

		for (ColliderComponent* collider : colliderComponents)
		{
			AABB bounds = collider->GetAABB();
			bounds.SetOffset(collider->GetParent()->GetWorldPosition());

			std::vector<AABBHit> hits;
			if (bounds.CollideWithRay(fromPos, direction, hits))
			{
				std::sort(hits.begin(), hits.end(), [](const AABBHit& left, const AABBHit& right) -> bool { return left.Distance < right.Distance; });
				const AABBHit& closestHit = hits[0];

				if (closestHit.Distance <= maxDistance)
				{
					Vector3 normal = bounds.GetRayHitNormal((Vector3)closestHit.Point);
					results.emplace_back(RayCastHitResult{collider->GetOwner(), closestHit.Point, normal, closestHit.Distance});

					count++;
				}
			}
		}

		if (count == 0)
			return 0;

		std::sort(results.begin(), results.end(), [](const RayCastHitResult& left, const RayCastHitResult& right) -> bool { return left.HitDistance < right.HitDistance; });

		return count;
	}
}