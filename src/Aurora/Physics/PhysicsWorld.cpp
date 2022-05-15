#include "PhysicsWorld.hpp"

#include "Aurora/Core/Profiler.hpp"
#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/Physics/RigidBodyComponent.hpp"
#include "Aurora/Framework/Physics/ColliderComponent.hpp"

#include "Aurora/Graphics/DShape.hpp"

#include "AABBTree.hpp"
#include "AABBUtil.hpp"

#include "Integration.hpp"

namespace Aurora
{
	//AABBTree<ColliderComponent> aabbTree(1);
	AABBTreeRaw aabbTreeRaw(1);

	PhysicsWorld::PhysicsWorld(Scene* scene) : m_Scene(scene), m_Accumulator(0), m_Time(0), m_DebugRender(false), m_Gravity(0, -20.0f, 0), m_UpdateRate(1.0 / 120.0)
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
			if (!collider->IsActive() || !collider->GetParent()->IsActive() || !collider->GetOwner()->IsActive())
				continue;

			AABB bounds = collider->GetAABB();
			bounds.SetOffset(collider->GetParent()->GetTransform().GetLocation());
			aabbTreeRaw.insertObject(collider, bounds);
		}

		ComponentView<RigidBodyComponent> bodyComponents = m_Scene->GetComponents<RigidBodyComponent>();
		for (RigidBodyComponent* rigidBodyComponent : bodyComponents)
		{
			if (rigidBodyComponent->IsKinematic() || !rigidBodyComponent->IsActive() || !rigidBodyComponent->GetOwner()->IsActive())
				continue;

			rigidBodyComponent->GetOwner()->FixedStep();
			rigidBodyComponent->AddAcceleration(m_Gravity * (float)m_UpdateRate);

			SceneComponent* parent = rigidBodyComponent->GetParent() != nullptr ? rigidBodyComponent->GetParent() : rigidBodyComponent->GetOwner()->GetRootComponent();
			std::vector<BoxColliderComponent*> colliders;
			parent->GetComponentsOfType(colliders);

			Transform& transform = rigidBodyComponent->GetOwner()->GetRootComponent()->GetTransform();

			Vector3 velocity = rigidBodyComponent->GetVelocity();
			velocity += rigidBodyComponent->GetAcceleration();

			if (rigidBodyComponent->GetFriction() > 0.0f)
			{
				velocity.x *= rigidBodyComponent->GetFriction();
				velocity.z *= rigidBodyComponent->GetFriction();
			}

			bool isMoving = glm::length2(velocity) > 0.0f;

			rigidBodyComponent->YCollided = false;

			if (isMoving)
			{
				for (BoxColliderComponent* collider : colliders)
				{
					// Predict bounding box
					AABB colliderBounds = collider->GetTransformedAABB();

					AABB predictedBounds = colliderBounds;
					predictedBounds.SetOffset(velocity * (float)m_UpdateRate);
					//AABB expandedAABB(predictedBounds.GetMin() - colliderBounds.GetSize() / 2.0f, predictedBounds.GetMax() + colliderBounds.GetSize() / 2.0f);

					//DShapes::Box(expandedAABB, Color::blue(), true, 1.0f);

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

					int a = 0;

					auto possibleColliders = aabbTreeRaw.queryOverlaps(collider, predictedBounds);
					for (auto collisionObject : possibleColliders)
					{
						a++;
						AABB boundsOther = collisionObject->GetAABB();
						boundsOther.SetOffset(collisionObject->GetParent()->GetWorldPosition());

						Vector3 contactPoint, contactNormal;
						float t_hit_near = 0.0f;

						if (AABBVsAABB(colliderBounds, boundsOther, velocity * (float)m_UpdateRate, contactPoint, contactNormal, t_hit_near))
						{
							collides.emplace_back(CollideData{collisionObject, boundsOther, contactPoint, contactNormal, t_hit_near});
						}
					}

					static int lastCount = 0;

					std::sort(collides.begin(), collides.end(), [](const CollideData& a, const CollideData& b)
					{
						return a.HitTime < b.HitTime;
					});

					if (lastCount != collides.size() && false)
					{
						lastCount = collides.size();

						AU_LOG_INFO("---------------");

						if (lastCount > 0)
							AU_LOG_INFO(velocity.y * (float)m_UpdateRate, " - ", collides.size(), ", ", glm::to_string(collides[0].Normal), ", ", a);
						else
							AU_LOG_WARNING(velocity.y * (float)m_UpdateRate, " - ", collides.size(), ", ", a);

						for (auto collisionObject : possibleColliders)
						{
							AU_LOG_INFO(collisionObject->GetOwner()->GetName());
						}

						for (const CollideData& data : collides)
						{
							AU_LOG_INFO(data.HitTime, " - ", glm::to_string(data.Normal));
						}
					}

					for (const CollideData& collision : collides)
					{
						Vector3 contactPoint, contactNormal;
						float t_hit_near = 0.0f;

						// FIXME: velocity should be multiplied by frame time, but if I do that I often tunnel through objects
						if (AABBVsAABB(colliderBounds, collision.Bounds, velocity * (float)m_UpdateRate, contactPoint, contactNormal, t_hit_near))
						{
							// Push back rigid bodies
							/*std::vector<RigidBodyComponent*> bodies;
							if (collision.Object->GetParent()->GetComponentsOfType(bodies))
							{
								RigidBodyComponent* collisionBody = bodies[0];

								if (!collisionBody->IsKinematic() && collisionBody->IsActive() && collisionBody->GetOwner()->IsActive())
								{
									// I think this is just a hack, and it should not work like this
									collisionBody->AddVelocity(-(contactNormal * glm::abs(velocity) * (1.0f - t_hit_near)));
								}
							}*/

							if (glm::abs(contactNormal.y) > 0)
							{
								rigidBodyComponent->YCollided = true;
							}

							velocity += contactNormal * glm::abs(velocity) * (1.0f - t_hit_near);

							/*if (rigidBodyComponent->GetFriction() > 0.0f)
							{
								velocity.x *= rigidBodyComponent->GetFriction();
								velocity.z *= rigidBodyComponent->GetFriction();
							}*/
						}
					}
				}
			}

			/*Vector3 location = transform.GetLocation();
			MotionIntegrators::ModifiedEuler(location, velocity, rigidBodyComponent->GetAcceleration(), (float)m_UpdateRate);
			transform.SetLocation(location);*/
			transform.SetLocation(transform.GetLocation() + velocity * (float)m_UpdateRate);

			rigidBodyComponent->SetVelocity(velocity);
			rigidBodyComponent->SetAcceleration({0, 0, 0});

			if (isMoving)
			{
				for (BoxColliderComponent* collider : colliders)
				{
					// Revert bounds back
					aabbTreeRaw.updateObject(collider, collider->GetTransformedAABB());
				}
			}
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