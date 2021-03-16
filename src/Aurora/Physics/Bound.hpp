#pragma once

#include "Ray.hpp"
#include "CollisionResult.hpp"

namespace Aurora
{
	class Bound
	{
	public:
		virtual ~Bound() = default;

		virtual int CollideWithRay(const Ray& ray, CollisionResults& results) const = 0;
	};
}