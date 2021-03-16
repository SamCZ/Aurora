#include "Ray.hpp"

namespace Aurora
{
	Ray::Ray() : Limit(DBL_MAX)
	{
	}

	Ray::Ray(const Vector3D & origin, const Vector3D & direction) : Origin(origin), Direction(direction), Limit(DBL_MAX)
	{
	}

	bool Ray::IntersectWithTriangle(const Vector3D & v0, const Vector3D & v1, const Vector3D & v2, double& distance)
	{
		double edge1X = v1.x - v0.x;
		double edge1Y = v1.y - v0.y;
		double edge1Z = v1.z - v0.z;

		double edge2X = v2.x - v0.x;
		double edge2Y = v2.y - v0.y;
		double edge2Z = v2.z - v0.z;

		double normX = ((edge1Y * edge2Z) - (edge1Z * edge2Y));
		double normY = ((edge1Z * edge2X) - (edge1X * edge2Z));
		double normZ = ((edge1X * edge2Y) - (edge1Y * edge2X));

		double dirDotNorm = Direction.x * normX + Direction.y * normY + Direction.z * normZ;

		double diffX = Origin.x - v0.x;
		double diffY = Origin.y - v0.y;
		double diffZ = Origin.z - v0.z;

		double sign;
		if (dirDotNorm > DBL_EPSILON)
		{
			sign = 1;
		}
		else if (dirDotNorm < -DBL_EPSILON)
		{
			sign = -1.0f;
			dirDotNorm = -dirDotNorm;
		}
		else
		{
			// ray and triangle/quad are parallel
			return false;
		}

		double diffEdge2X = ((diffY * edge2Z) - (diffZ * edge2Y));
		double diffEdge2Y = ((diffZ * edge2X) - (diffX * edge2Z));
		double diffEdge2Z = ((diffX * edge2Y) - (diffY * edge2X));

		double dirDotDiffxEdge2 = sign * (Direction.x * diffEdge2X
										 + Direction.y * diffEdge2Y
										 + Direction.z * diffEdge2Z);

		if (dirDotDiffxEdge2 >= 0.0f)
		{
			diffEdge2X = ((edge1Y * diffZ) - (edge1Z * diffY));
			diffEdge2Y = ((edge1Z * diffX) - (edge1X * diffZ));
			diffEdge2Z = ((edge1X * diffY) - (edge1Y * diffX));

			double dirDotEdge1xDiff = sign * (Direction.x * diffEdge2X
											 + Direction.y * diffEdge2Y
											 + Direction.z * diffEdge2Z);

			if (dirDotEdge1xDiff >= 0.0)
			{
				if (dirDotDiffxEdge2 + dirDotEdge1xDiff <= dirDotNorm)
				{
					double diffDotNorm = -sign * (diffX * normX + diffY * normY + diffZ * normZ);
					if (diffDotNorm >= 0.0)
					{
						// ray intersects triangle
						// fill in.
						double inv = 1.0 / dirDotNorm;
						distance = diffDotNorm * inv;
						return true;
					}
				}
			}
		}

		return false;
	}
}