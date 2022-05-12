#pragma once

#include "Types.hpp"

// Most of the code taken from bullet3

namespace Aurora
{
	inline int phOutcode(const phVector3& p, const phVector3& halfExtent)
	{
		return (p.x < -halfExtent.x ? 0x01 : 0x0) |
			(p.x > halfExtent.x ? 0x08 : 0x0) |
			(p.y < -halfExtent.y ? 0x02 : 0x0) |
			(p.y > halfExtent.y ? 0x10 : 0x0) |
			(p.z < -halfExtent.z ? 0x4 : 0x0) |
			(p.z > halfExtent.z ? 0x20 : 0x0);
	}

	inline bool phRayAabb(const phVector3& rayFrom,
		const phVector3& rayTo,
		const phVector3& aabbMin,
		const phVector3& aabbMax,
		phScalar& param, phVector3& normal)
	{
		phVector3 aabbHalfExtent = (aabbMax - aabbMin) * phScalar(0.5);
		phVector3 aabbCenter = (aabbMax + aabbMin) * phScalar(0.5);
		phVector3 source = rayFrom - aabbCenter;
		phVector3 target = rayTo - aabbCenter;

		int sourceOutcode = phOutcode(source, aabbHalfExtent);
		int targetOutcode = phOutcode(target, aabbHalfExtent);

		if ((sourceOutcode & targetOutcode) == 0x0)
		{
			phScalar lambda_enter = phScalar(0.0);
			phScalar lambda_exit = param;
			phVector3 r = target - source;
			int i;
			phScalar normSign = 1;
			phVector3 hitNormal(0, 0, 0);
			int bit = 1;

			for (int j = 0; j < 2; j++)
			{
				for (i = 0; i != 3; ++i)
				{
					if (sourceOutcode & bit)
					{
						phScalar lambda = (-source[i] - aabbHalfExtent[i] * normSign) / r[i];
						if (lambda_enter <= lambda)
						{
							lambda_enter = lambda;
							hitNormal = {0, 0, 0};
							hitNormal[i] = normSign;
						}
					}
					else if (targetOutcode & bit)
					{
						phScalar lambda = (-source[i] - aabbHalfExtent[i] * normSign) / r[i];
						lambda_exit = std::min(lambda_exit, lambda);
					}
					bit <<= 1;
				}
				normSign = phScalar(-1.);
			}
			if (lambda_enter <= lambda_exit)
			{
				param = lambda_enter;
				normal = hitNormal;
				return true;
			}
		}
		return false;
	}

}