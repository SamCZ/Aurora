#pragma once

#include "Types.hpp"
#include "Aurora/Framework/Transform.hpp"

// Most of the code taken from bullet3

#define ANGULAR_MOTION_THRESHOLD phScalar(0.5) * SIMD_HALF_PI

namespace Aurora
{
	class phTransformUtil
	{
	public:
		static void IntegrateTransform(const Transform& curTrans, const phVector3& linvel, const phVector3& angvel, phScalar timeStep, Transform& predictedTransform)
		{
			predictedTransform.SetLocation(curTrans.GetLocation() + linvel * timeStep);

			//Exponential map
			//google for "Practical Parameterization of Rotations Using the Exponential Map", F. Sebastian Grassia

			phVector3 axis;
			phScalar fAngle2 = glm::length2(angvel);
			phScalar fAngle = 0;
			if (fAngle2 > SIMD_EPSILON)
			{
				fAngle = glm::sqrt(fAngle2);
			}

			//limit the angular motion
			if (fAngle * timeStep > ANGULAR_MOTION_THRESHOLD)
			{
				fAngle = ANGULAR_MOTION_THRESHOLD / timeStep;
			}

			if (fAngle < phScalar(0.001))
			{
				// use Taylor's expansions of sync function
				axis = angvel * (phScalar(0.5) * timeStep - (timeStep * timeStep * timeStep) * (phScalar(0.020833333333)) * fAngle * fAngle);
			}
			else
			{
				// sync(fAngle) = sin(c*fAngle)/t
				axis = angvel * (glm::sin(phScalar(0.5) * fAngle * timeStep) / fAngle);
			}
			phQuaternion dorn(axis.x, axis.y, axis.z, glm::cos(fAngle * timeStep * phScalar(0.5)));
			phQuaternion orn0 = curTrans.GetRotationQuaternion();

			phQuaternion predictedOrn = glm::normalize(dorn * orn0);

			if (glm::length2(predictedOrn) > SIMD_EPSILON)
			{
				predictedTransform.SetRotation(predictedOrn);
			}
			else
			{
				predictedTransform.SetRotation(curTrans.GetRotation());
			}
		}
	};
}