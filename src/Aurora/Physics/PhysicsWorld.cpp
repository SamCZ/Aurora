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
	AABBTree<ColliderComponent> aabbTree(1);

	PhysicsWorld::PhysicsWorld(Scene* scene) : m_Scene(scene), m_Accumulator(0), m_Time(0), m_DebugRender(false)
	{

	}

	void PhysicsWorld::Update(double frameTime)
	{
		CPU_DEBUG_SCOPE("PhysicsWorld");

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

		if (IsDebugRender())
		{
			for (const auto& node : aabbTree.GetNodes())
			{
				if (node.ParentNodeIndex == AABB_NULL_NODE || !node.IsLeaf())
					continue;

				DShapes::Box(node.Bounds, Color::green(), true, 1.0f, 0, false);
			}
		}
	}

	void PhysicsWorld::RunPhysics(double time, double timeStep)
	{
		ComponentView<ColliderComponent> colliderComponents = m_Scene->GetComponents<ColliderComponent>();
		aabbTree = AABBTree<ColliderComponent>(colliderComponents.size());

		for (ColliderComponent* collider : colliderComponents)
		{
			AABB bounds = collider->GetAABB();
			bounds.SetOffset(collider->GetParent()->GetTransform().GetLocation());
			aabbTree.InsertObject(collider, bounds);
		}

		ComponentView<RigidBodyComponent> bodyComponents = m_Scene->GetComponents<RigidBodyComponent>();
		for (RigidBodyComponent* rigidBodyComponent : bodyComponents)
		{
			if (rigidBodyComponent->IsKinematic())
				continue;

			SceneComponent* parent = rigidBodyComponent->GetParent() != nullptr ? rigidBodyComponent->GetParent() : rigidBodyComponent->GetOwner()->GetRootComponent();
			std::vector<BoxColliderComponent*> colliders;
			parent->GetComponentsOfType(colliders);
			BoxColliderComponent* collider = colliders[0];

			rigidBodyComponent->SetLinearVelocity({0, -10, 0});

			if (false)
			{ // new physics
				Transform predictedTrans;
				rigidBodyComponent->PredictIntegratedTransform(timeStep, predictedTrans);

				AABB bounds = collider->GetAABB();
				bounds.SetOffset(rigidBodyComponent->GetWorldTransform().GetLocation());
				aabbTree.UpdateObject(collider, bounds);

				phScalar squareMotion = glm::length2(predictedTrans.GetLocation() - rigidBodyComponent->GetWorldTransform().GetLocation());

				if (squareMotion > 0.0)
				{
					Transform modifiedPredictedTrans = predictedTrans;
					modifiedPredictedTrans.SetRotation(rigidBodyComponent->GetWorldTransform().GetRotation());

					for (auto collisionObject : aabbTree.QueryOverlaps(collider, bounds))
					{
						phVector3 collisionObjectAabbMin, collisionObjectAabbMax;
						collisionObject->GetAabb(collisionObject->GetParent()->GetTransform(), collisionObjectAabbMin, collisionObjectAabbMax);

						collisionObjectAabbMin += bounds.GetMin();
						collisionObjectAabbMax += bounds.GetMax();
						//AabbExpand(collisionObjectAabbMin, collisionObjectAabbMax, castShapeAabbMin, castShapeAabbMax);

						phScalar hitLambda = phScalar(1.);  //could use resultCallback.m_closestHitFraction, but needs testing
						phVector3 hitNormal;

						if (phRayAabb(rigidBodyComponent->GetWorldTransform().GetLocation(), predictedTrans.GetLocation(), collisionObjectAabbMin, collisionObjectAabbMax, hitLambda, hitNormal))
						{
							rigidBodyComponent->PredictIntegratedTransform(timeStep * hitLambda, predictedTrans);
							rigidBodyComponent->GetWorldTransform().SetFromMatrixNoScale(predictedTrans.GetTransform());
						}

					}

					//ConvexSweepTest(&tmpSphere, body->getWorldTransform(), modifiedPredictedTrans, sweepResults);
				}

				rigidBodyComponent->GetWorldTransform().SetFromMatrixNoScale(predictedTrans.GetTransform());
			}
			else
			{
				Transform& transform = rigidBodyComponent->GetOwner()->GetRootComponent()->GetTransform();

				Vector3 velocity = {0, -timeStep * 10.0f, 0};
				//transform.AddLocation(velocity);

				AABB bounds = collider->GetAABB();
				bounds.SetOffset(collider->GetParent()->GetWorldPosition());
				aabbTree.UpdateObject(collider, bounds);

				transform.AddLocation(velocity);

				for (auto collisionObject : aabbTree.QueryOverlaps(collider, bounds))
				{
					if (collisionObject->GetParent()->GetName() == "Camera") continue;

					//AU_LOG_INFO("Overlap ", parent->GetOwner()->GetName(), " with ", collisionObject->GetOwner()->GetName());

					AABB boundsOther = collisionObject->GetAABB();
					boundsOther.SetOffset(collisionObject->GetParent()->GetWorldPosition());

					/*for (int axis = 0; axis < 3; ++axis)
					{
						float axisVelocity = velocity[axis];
						if (glm::abs(axisVelocity) == 0) continue;

						Vector3 localAxisVelocity = { 0, 0, 0 };
						localAxisVelocity[axis] = axisVelocity;

						bounds = collider->GetAABB();
						bounds.SetOffset(collider->GetParent()->GetWorldPosition() - velocity + localAxisVelocity);

						AABB intersection = bounds.Intersection(boundsOther);

						Vector3 objPos = transform.GetLocation();
						objPos[axis] += intersection.GetSize()[axis];
						transform.SetLocation(objPos);
					}*/

					/*phVector3 collisionObjectAabbMin, collisionObjectAabbMax;
					collisionObject->GetAabb(collisionObject->GetParent()->GetTransform(), collisionObjectAabbMin, collisionObjectAabbMax);

					collisionObjectAabbMin += bounds.GetMin();
					collisionObjectAabbMax += bounds.GetMax();

					AABB boundsOther(collisionObjectAabbMin, collisionObjectAabbMax);*/

					/*Vector3 ray_origin = bounds.GetOrigin();
					Vector3 ray_dir = velocity;
					float maxDistance = glm::length(ray_dir);

					std::vector<AABBHit> hits;
					if (boundsOther.CollideWithRay(ray_origin, ray_dir, hits))
					{
						std::sort(hits.begin(), hits.end(), [](const AABBHit& left, const AABBHit& right) -> bool { return left.Distance < right.Distance; });

						const AABBHit& closestHit = hits[0];

						Vector3 normal = glm::sign((Vector3)closestHit.Point - boundsOther.GetOrigin());

						//velocity *= -normal * 1.0f - (float(closestHit.Distance) / maxDistance);
						transform.AddLocation(velocity * -normal * 1.0f - (float(closestHit.Distance) / maxDistance));
					}*/

					AABB intersection = bounds.Intersection(boundsOther);

					//AU_LOG_INFO("Overlap ", parent->GetOwner()->GetName(), " with ", overlap->GetOwner()->GetName());
					transform.SetLocation(transform.GetLocation() + Vector3(0, intersection.GetHeight(), 0));
				}



			}


			{
				AABB bounds = collider->GetAABB();
				bounds.SetOffset(collider->GetParent()->GetWorldPosition());
				aabbTree.UpdateObject(collider, bounds);
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
					// FIXME: this normal calculation is wrong
					Vector3 normal = glm::sign((Vector3)closestHit.Point - bounds.GetOrigin());

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