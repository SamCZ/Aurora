#pragma once

#include "Common.hpp"

#include <iostream>
#include <limits>

#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>

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

namespace Aurora::Math
{
	template<typename T>
	static constexpr FINLINE T Lerp(T min, T max, T alpha)
	{
		return min + (max - min) * alpha;
	}

	template<typename T, typename Other>
	static constexpr FINLINE T Min(T left, Other right)
	{
		return left <= right ? left : right;
	}

	template<typename T, typename Other>
	static constexpr FINLINE T Max(T left, Other right)
	{
		return left >= right ? left : right;
	}

	template<typename T, typename Min, typename Max>
	static constexpr FINLINE T Clamp(T value, Min min, Max max)
	{
		if (value > max)
			return max;

		if (value < min)
			return min;

		return value;
	}

	template<typename T, typename Min, typename Max>
	static constexpr FINLINE T WrapAround(T value, Min min, Max max)
	{
		if (value > max)
			return (T)min;

		if (value < min)
			return (T)max;

		return value;
	}

	template<typename T>
	static constexpr FINLINE T Saturate(T value)
	{
		return Clamp(value, T(0), T(1));
	};

	template<typename T>
	static constexpr FINLINE T Abs(T value)
	{
		return (value >= T(0)) ? value : -value;
	};
}

namespace glm
{
	AU_API glm::vec3 SmoothDamp(const glm::vec3& current, glm::vec3 target, glm::vec3& currentVelocity, double smoothTime, double maxSpeed, double deltaTime);
	AU_API glm::vec3 SmoothDamp(const glm::vec3& current, const glm::vec3& target, glm::vec3& currentVelocity, double smoothTime, double deltaTime);

	template<typename T> requires(std::is_floating_point<T>::value)
	inline vec<3, T, defaultp> MoveTowards(const vec<3, T, defaultp>& current, const vec<3, T, defaultp>& target, T maxDistanceDelta)
	{
		// avoid vector ops because current scripting backends are terrible at inlining
		T toVector_x = target.x - current.x;
		T toVector_y = target.y - current.y;
		T toVector_z = target.z - current.z;

		T sqdist = toVector_x * toVector_x + toVector_y * toVector_y + toVector_z * toVector_z;

		if (sqdist == 0 || (maxDistanceDelta >= 0 && sqdist <= maxDistanceDelta * maxDistanceDelta))
			return target;
		double dist = glm::sqrt(sqdist);

		return {current.x + toVector_x / dist * maxDistanceDelta,
		        current.y + toVector_y / dist * maxDistanceDelta,
		        current.z + toVector_z / dist * maxDistanceDelta};
	}

	// Taken from HazelDev
	AU_API bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}