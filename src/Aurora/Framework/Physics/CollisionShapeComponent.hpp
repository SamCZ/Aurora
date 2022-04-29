#pragma once

#include "Aurora/Framework/Actor.hpp"
#include "Aurora/Framework/SceneComponent.hpp"
#include <btBulletDynamicsCommon.h>

namespace Aurora
{
	enum class ECollisionShape : uint8_t
	{
		Box,
		Sphere,
		Capsule,
		ConvexMesh,
		Mesh
	};

	class AU_API CollisionShapeComponent : public SceneComponent
	{
	private:

	public:
		CLASS_OBJ(CollisionShapeComponent, SceneComponent);

		void BeginPlay() override;
	};
}
