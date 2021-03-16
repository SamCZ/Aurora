#pragma once

#include "Aurora/Core/Vector.hpp"

namespace Aurora::MotionIntegrators
{
	inline void Verlet(Vector3D& pos, Vector3D& velocity, const Vector3D& acceleration, double delta)
	{
		double halfDelta = delta * 0.5;
		pos += velocity * halfDelta;
		velocity += acceleration * delta;
		pos += velocity * halfDelta;
	}

	inline void ForestRuth(Vector3D& pos, Vector3D& velocity, const Vector3D& acceleration, double delta)
	{
		static const double frCoefficient = 1.0 / (2.0 - pow(2.0,1.0/3.0));
		static const double frComplement = 1.0 - 2.0 * frCoefficient;
		Verlet(pos, velocity, acceleration, delta*frCoefficient);
		Verlet(pos, velocity, acceleration, delta*frComplement);
		Verlet(pos, velocity, acceleration, delta*frCoefficient);
	}

	inline void ModifiedEuler(Vector3D& pos, Vector3D& velocity, const Vector3D& acceleration, double delta)
	{
		velocity += acceleration * delta;
		pos += velocity * delta;
	}
}