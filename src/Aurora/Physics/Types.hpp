#pragma once

#include "Aurora/Core/Vector.hpp"

#ifdef AU_PHYSICS_USE_DOUBLE
typedef double phScalar;
typedef glm::dvec3 phVector3;
typedef glm::dquat phQuaternion;
	#define SIMD_EPSILON DBL_EPSILON
	#define SIMD_INFINITY DBL_MAX
#else
typedef float phScalar;
typedef glm::vec3 phVector3;
typedef glm::quat phQuaternion;
typedef glm::mat3x3 phMatrix3;
	#define SIMD_EPSILON FLT_EPSILON
	#define SIMD_INFINITY FLT_MAX
#endif

#define SIMD_PI phScalar(3.1415926535897932384626433832795029)
#define SIMD_2_PI (phScalar(2.0) * SIMD_PI)
#define SIMD_HALF_PI (SIMD_PI * phScalar(0.5))
#define SIMD_RADS_PER_DEG (SIMD_2_PI / btScalar(360.0))
#define SIMD_DEGS_PER_RAD (btScalar(360.0) / SIMD_2_PI)