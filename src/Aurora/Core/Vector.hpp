#pragma once

#include <iostream>
#include <limits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wvolatile"

#define GLM_EXT_INCLUDED
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/hash.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#pragma GCC diagnostic pop

typedef glm::vec4 Vector4;
typedef glm::vec3 Vector3;
typedef glm::vec2 Vector2;

typedef glm::dvec4 Vector4D;
typedef glm::dvec3 Vector3D;
typedef glm::dvec2 Vector2D;

typedef glm::ivec4 Vector4i;
typedef glm::ivec3 Vector3i;
typedef glm::ivec2 Vector2i;

typedef glm::uvec4 Vector4ui;
typedef glm::uvec3 Vector3ui;
typedef glm::uvec2 Vector2ui;

typedef glm::u8vec2 Vector2B;

typedef glm::mat4 Matrix4;
typedef glm::mat3 Matrix3;

typedef glm::quat Quaternion;

namespace glm
{
	static glm::dvec3 SmoothDamp(const glm::dvec3& current, glm::dvec3 target, glm::dvec3& currentVelocity, double smoothTime, double maxSpeed, double deltaTime)
	{
		double output_x = 0;
		double output_y = 0;
		double output_z = 0;

		// Based on Game Programming Gems 4 Chapter 1.10
		smoothTime = glm::max(0.0001, smoothTime);
		double omega = 2.0 / smoothTime;

		double x = omega * deltaTime;
		double exp = 1.0 / (1.0 + x + 0.48 * x * x + 0.235 * x * x * x);

		double change_x = current.x - target.x;
		double change_y = current.y - target.y;
		double change_z = current.z - target.z;
		glm::dvec3 originalTo = target;

		// Clamp maximum speed
		double maxChange = maxSpeed * smoothTime;

		double maxChangeSq = maxChange * maxChange;
		double sqrmag = change_x * change_x + change_y * change_y + change_z * change_z;
		if (sqrmag > maxChangeSq)
		{
			double mag = glm::sqrt(sqrmag);
			change_x = change_x / mag * maxChange;
			change_y = change_y / mag * maxChange;
			change_z = change_z / mag * maxChange;
		}

		target.x = current.x - change_x;
		target.y = current.y - change_y;
		target.z = current.z - change_z;

		double temp_x = (currentVelocity.x + omega * change_x) * deltaTime;
		double temp_y = (currentVelocity.y + omega * change_y) * deltaTime;
		double temp_z = (currentVelocity.z + omega * change_z) * deltaTime;

		currentVelocity.x = (currentVelocity.x - omega * temp_x) * exp;
		currentVelocity.y = (currentVelocity.y - omega * temp_y) * exp;
		currentVelocity.z = (currentVelocity.z - omega * temp_z) * exp;

		output_x = target.x + (change_x + temp_x) * exp;
		output_y = target.y + (change_y + temp_y) * exp;
		output_z = target.z + (change_z + temp_z) * exp;

		// Prevent overshooting
		double origMinusCurrent_x = originalTo.x - current.x;
		double origMinusCurrent_y = originalTo.y - current.y;
		double origMinusCurrent_z = originalTo.z - current.z;
		double outMinusOrig_x = output_x - originalTo.x;
		double outMinusOrig_y = output_y - originalTo.y;
		double outMinusOrig_z = output_z - originalTo.z;

		if (origMinusCurrent_x * outMinusOrig_x + origMinusCurrent_y * outMinusOrig_y + origMinusCurrent_z * outMinusOrig_z > 0)
		{
			output_x = originalTo.x;
			output_y = originalTo.y;
			output_z = originalTo.z;

			currentVelocity.x = (output_x - originalTo.x) / deltaTime;
			currentVelocity.y = (output_y - originalTo.y) / deltaTime;
			currentVelocity.z = (output_z - originalTo.z) / deltaTime;
		}

		return {output_x, output_y, output_z};
	}

	static glm::dvec3 SmoothDamp(const glm::dvec3& current, const glm::dvec3& target, glm::dvec3& currentVelocity, double smoothTime, double deltaTime)
	{
		return SmoothDamp(current, target, currentVelocity, smoothTime, std::numeric_limits<double>::max(), deltaTime);
	}

	static glm::dvec3 MoveTowards(const glm::dvec3& current, const glm::dvec3& target, double maxDistanceDelta)
	{
		// avoid vector ops because current scripting backends are terrible at inlining
		double toVector_x = target.x - current.x;
		double toVector_y = target.y - current.y;
		double toVector_z = target.z - current.z;

		double sqdist = toVector_x * toVector_x + toVector_y * toVector_y + toVector_z * toVector_z;

		if (sqdist == 0 || (maxDistanceDelta >= 0 && sqdist <= maxDistanceDelta * maxDistanceDelta))
			return target;
		double dist = glm::sqrt(sqdist);

		return glm::dvec3(current.x + toVector_x / dist * maxDistanceDelta,
						  current.y + toVector_y / dist * maxDistanceDelta,
						  current.z + toVector_z / dist * maxDistanceDelta);
	}
}