#pragma once

#include "Aurora/Graphics/DShape.hpp"
#include "Aurora/Framework/Physics/RigidBodyComponent.hpp"
#include "Aurora/Framework/Physics/ColliderComponent.hpp"
#include "AABBTree.hpp"


namespace Aurora
{
	namespace BroadPhase
	{
		static bool FromAABB(BoxColliderComponent* current, const AABBTree<ColliderComponent>& bvhTree, Vector3& velocity, bool* axes, double updateRate)
		{
			AABB currentBounds = current->GetTransformedAABB();

			bool collision = false;

			Vector3 offset = {0, 0, 0};
			for (uint8_t axis = 0; axis < 3; ++axis)
			{
				/*if (glm::abs(velocity[axis]) < glm::epsilon<float>())
					continue;*/

				AABB predictedBounds = currentBounds;
				offset[axis] = velocity[axis] * (float)updateRate;


				/*if (axis == 0 || axis == 2)
				{
					offset[0] = velocity[0] * (float)updateRate;
					offset[2] = velocity[2] * (float)updateRate;
				}*/

				predictedBounds.SetOffset(offset);
				AABB encapsulatedBounds = predictedBounds.Merge(currentBounds);

				//DShapes::Box(encapsulatedBounds, Color::blue(), true, 1.0f);

				auto possibleColliders = bvhTree.QueryOverlaps(current, encapsulatedBounds);

				for (ColliderComponent* collisionObject : possibleColliders)
				{
					AABB otherBounds = collisionObject->GetTransformedAABB();

					// Resolve proxy first
					if (ProxyColliderComponent* proxy = ProxyColliderComponent::SafeCast(collisionObject))
					{
						if (!proxy->CollideWith(currentBounds, encapsulatedBounds, velocity, updateRate, axis))
						{
							continue;
						}
					}

					//DShapes::Box(collisionObject->GetTransformedAABB() * 1.1f, Color::green(), true, 1.0f);

					velocity[axis] = 0;
					offset[axis] = 0;

					Vector3 pos = {0, 0, 0};
					pos[axis] = -velocity[axis] * (float)updateRate;
					current->GetOwner()->GetTransform().AddLocation(pos);

					if (axes)
					{
						axes[axis] = true;
					}

					collision = true;
					break;
				}
			}

			return collision;
		}
	}
}