#pragma once

#include "Types.hpp"
#include "Aurora/Graphics/DShape.hpp"

namespace Aurora
{
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

		// Replaced instead of https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_Rectangles.cpp#L123
		contactNormal = aabb.GetRayHitNormal(contactPoint);
		return true;
	}

	bool AABBVsAABB(const AABB& firstAABB, const AABB& secondAABB, const Vector3& velocity, Vector3& contactPoint, Vector3& contactNormal, float& t_hit_near)
	{
		if (glm::length2(velocity) == 0)
			return false;

		AABB expandedAABB(secondAABB.GetMin() - firstAABB.GetSize() / 2.0f, secondAABB.GetMax() + firstAABB.GetSize() / 2.0f);

		//DShapes::Box(expandedAABB, Color::green(), true, 1.0f);

		Vector3 origin = firstAABB.GetOrigin();
		return RayAABB(origin, velocity, expandedAABB, contactPoint, contactNormal, t_hit_near) && t_hit_near >= 0.0f && t_hit_near < 1.0f;
	}
}