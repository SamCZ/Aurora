#pragma once

#include <iostream>
#include <limits>

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wvolatile"
#endif

#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

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
	glm::dvec3 SmoothDamp(const glm::dvec3& current, glm::dvec3 target, glm::dvec3& currentVelocity, double smoothTime, double maxSpeed, double deltaTime);
	glm::dvec3 SmoothDamp(const glm::dvec3& current, const glm::dvec3& target, glm::dvec3& currentVelocity, double smoothTime, double deltaTime);
	glm::dvec3 MoveTowards(const glm::dvec3& current, const glm::dvec3& target, double maxDistanceDelta);

	// Taken from HazelDev
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}