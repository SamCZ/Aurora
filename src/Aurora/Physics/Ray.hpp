#pragma once

#include <Aurora/Core/Vector.hpp>

namespace Aurora
{
	class Ray
	{
	public:
		Vector3D Origin = {};
		Vector3D Direction = {};

		float Limit;

		Ray();
		Ray(const Vector3D& origin, const Vector3D& direction);

		bool IntersectWithTriangle(const Vector3D& v0, const Vector3D& v1, const Vector3D& v2, double& distance);
	};
}
