#pragma once

#include "Types.hpp"

namespace Aurora::MotionIntegrators
{
	void Verlet(phVector3& pos, phVector3& velocity, const phVector3& acceleration, phScalar delta)
	{
		phScalar halfDelta = delta * phScalar(0.5);
		pos += velocity * halfDelta;
		velocity += acceleration * delta;
		pos += velocity * halfDelta;
	}

	void ForestRuth(phVector3& pos, phVector3& velocity, const phVector3& acceleration, phScalar delta)
	{
		static const phScalar frCoefficient = phScalar(1.0) / (phScalar(2.0) - glm::pow(phScalar(2.0), phScalar(1.0 / 3.0)));
		static const phScalar frComplement = phScalar(1.0) - phScalar(2.0) * frCoefficient;
		Verlet(pos, velocity, acceleration, delta * frCoefficient);
		Verlet(pos, velocity, acceleration, delta * frComplement);
		Verlet(pos, velocity, acceleration, delta * frCoefficient);
	}

	void ModifiedEuler(phVector3& pos, phVector3& velocity, const phVector3& acceleration, phScalar delta)
	{
		velocity += acceleration * delta;
		pos += velocity * delta;
	}
}