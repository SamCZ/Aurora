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

	PhysicsWorld::PhysicsWorld(Scene* scene) : m_Scene(scene), m_Accumulator(0), m_Time(0), m_DebugRender(false), m_Gravity(0, -10.0f, 0), m_UpdateRate(1.0 / 120.0)
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

	static bool RayAABB(const Vector3& rayOrigin, const Vector3& rayDirection, const AABB& aabb, Vector3& contactPoint, Vector3& contactNormal, float& t_hit_near)
	{
		Vector3 invDir = 1.0f / rayDirection;

		Vector3 t_near = (aabb.GetMin() - rayOrigin) * invDir;
		Vector3 t_far = (aabb.GetMax() - rayOrigin) * invDir;

		if (glm::any(glm::isnan(t_near))) return false;
		if (glm::any(glm::isnan(t_far))) return false;

		// Sort distances
		if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
		if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);
		if (t_near.z > t_far.z) std::swap(t_near.z, t_far.z);

		if (t_near.x > t_far.y || t_near.y > t_far.x) return false;
		if (t_near.x > t_far.z || t_near.z > t_far.x) return false;

		// Closest 'time' will be the first contact
		t_hit_near = std::max(t_near.x, std::max(t_near.y, t_near.z));

		// Furthest 'time' is contact on opposite side of target
		float t_hit_far = std::min(t_far.x, std::min(t_far.y, t_far.z));

		// Reject if ray direction is pointing away from object
		if (t_hit_far < 0)
			return false;

		// Contact point of collision from parametric line equation
		contactPoint = rayOrigin + t_hit_near * rayDirection;

		contactNormal = aabb.GetRayHitNormal(contactPoint);
		return true;
	}

	bool AABBVsAABB(const AABB& firstAABB, const AABB& secondAABB, const Vector3& velocity, Vector3& contactPoint, Vector3& contactNormal, float& t_hit_near)
	{
		if (glm::length2(velocity) == 0)
			return false;

		AABB expandedAABB(secondAABB.GetMin() - firstAABB.GetSize() / 2.0f, secondAABB.GetMax() + firstAABB.GetSize() / 2.0f);

		Vector3 origin = firstAABB.GetOrigin();
		return RayAABB(origin, velocity, expandedAABB, contactPoint, contactNormal, t_hit_near) && t_hit_near >= 0.0f && t_hit_near < 1.0f;
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

			for (BoxColliderComponent* collider : colliders)
			{
				// Predict bounding box
				AABB colliderBounds = collider->GetAABB();
				AABB predictedBounds = colliderBounds;

				colliderBounds.SetOffset(transform.GetLocation());

				predictedBounds.SetOffset(transform.GetLocation() + velocity * (float)m_UpdateRate);
				aabbTreeRaw.updateObject(collider, predictedBounds);

				struct CollideData
				{
					ColliderComponent* Object;
					AABB Bounds;
					Vector3 Point;
					Vector3 Normal;
					float HitTime;
				};

				std::vector<CollideData> collides;

				for (auto collisionObject : aabbTreeRaw.queryOverlaps(collider, predictedBounds))
				{
					AABB boundsOther = collisionObject->GetAABB();
					boundsOther.SetOffset(collisionObject->GetParent()->GetWorldPosition());

					Vector3 contactPoint, contactNormal;
					float t_hit_near = 0.0f;
					if (AABBVsAABB(colliderBounds, boundsOther, velocity * (float)m_UpdateRate, contactPoint, contactNormal, t_hit_near))
					{
						collides.emplace_back(CollideData{collisionObject, boundsOther, contactPoint, contactNormal, t_hit_near});
					}
				}

				std::sort(collides.begin(), collides.end(), [](const CollideData& a, const CollideData& b)
				{
					return a.HitTime < b.HitTime;
				});

				for (const CollideData& collision : collides)
				{
					Vector3 contactPoint, contactNormal;
					float t_hit_near = 0.0f;
					if (AABBVsAABB(colliderBounds, collision.Bounds, velocity * (float)m_UpdateRate, contactPoint, contactNormal, t_hit_near))
					{
						velocity += contactNormal * glm::abs(velocity) * (1.0f - t_hit_near);
					}
				}

				transform.SetLocation(transform.GetLocation() + velocity * (float)m_UpdateRate);

				// Revert bounds back
				AABB bounds = collider->GetAABB();
				bounds.SetOffset(collider->GetParent()->GetWorldPosition());
				aabbTreeRaw.updateObject(collider, bounds);
			}

			rigidBodyComponent->SetVelocity(velocity);
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