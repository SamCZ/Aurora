#include "Scene.hpp"

#include <Aurora/Physics/CollisionMatrix.hpp>

namespace Aurora
{
	std::optional<CollisionResult> Scene::RayCast(const Ray &ray, const Layer &layer)
	{
		CollisionResults results;

		int collisionCount = 0;

		for(auto* cmp : m_SceneComponents) {
			if(!cmp->IsRootComponent()) {
				continue;
			}

			Actor* actor = cmp->GetOwner();

			if(!cmp->GetBody().HasCollider()) {
				continue;
			}

			if(!CollisionMatrix::CanCollide(cmp->GetLayer(), layer)) {
				//continue;
			}

			// TODO: Fix collision with camera
			if(cmp->Name == "Camera") continue;

			auto& bounds = cmp->GetBody().GetTransformedBounds();

			results.SetCurrentObject(actor);
			collisionCount += bounds.CollideWithRay(ray, results);
		}

		if(collisionCount > 0) {
			return results.GetClosestCollision();
		}

		return std::nullopt;
	}
}