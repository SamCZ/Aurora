#pragma once

#include "Aurora/Core/Math.hpp"

namespace Aurora
{
	struct Ray
	{
		Vector3 Origin;
		Vector3 Direction;

		inline Ray(const Vector3& origin, const Vector3& direction) : Origin(origin), Direction(direction) {}
		inline Ray() : Origin(0, 0, 0), Direction(0, 0, 0) {}
	};
}