#include "AABB.hpp"

#include "Aurora/Logger/Logger.hpp"

namespace Aurora
{
	AABB::AABB() : m_Min(), m_Max(), m_SurfaceArea(0)
	{

	}

	AABB::AABB(const Vector3 &min, const Vector3 &max) : m_Min(min), m_Max(max), m_SurfaceArea(0)
	{
		m_SurfaceArea = CalculateSurfaceArea();
	}

	AABB::~AABB() = default;

	AABB AABB::FromExtent(const Vector3 &origin, const Vector3 &extent)
	{
		return AABB(origin - extent, origin + extent);
	}

	const Vector3 &AABB::GetMin() const
	{
		return m_Min;
	}

	const Vector3 &AABB::GetMax() const
	{
		return m_Max;
	}

	void AABB::SetOffset(const Vector3 &offset)
	{
		m_Min += offset;
		m_Max += offset;
	}

	void AABB::SetOffset(double x, double y, double z)
	{
		SetOffset({x, y, z});
	}

	void AABB::Set(const Vector3& min, const Vector3& max)
	{
		m_Min = min;
		m_Max = max;

		m_SurfaceArea = CalculateSurfaceArea();
	}

	void AABB::Extend(const Vector3 &point)
	{
		CheckMinMax(m_Min, m_Max, point);

		m_SurfaceArea = CalculateSurfaceArea();
	}

	bool AABB::IntersectsWith(const AABB &other) const
	{
		return other.m_Max.x > m_Min.x && other.m_Min.x < m_Max.x ? (other.m_Max.y > m_Min.y && other.m_Min.y < m_Max.y ? other.m_Max.z > m_Min.z && other.m_Min.z < m_Max.z : false) : false;
	}

	void AABB::CheckMinMax(Vector3 &min, Vector3 &max, const Vector3 &point)
	{
		if (point.x < min.x)
		{
			min.x = point.x;
		}
		if (point.x > max.x)
		{
			max.x = point.x;
		}
		if (point.y < min.y)
		{
			min.y = point.y;
		}
		if (point.y > max.y)
		{
			max.y = point.y;
		}
		if (point.z < min.z)
		{
			min.z = point.z;
		}
		if (point.z > max.z)
		{
			max.z = point.z;
		}
	}

	Vector3 AABB::GetOrigin() const
	{
		return m_Min + GetExtent();
	}

	Vector3 AABB::GetExtent() const
	{
		return (m_Max - m_Min) / 2.0f;
	}

	float AABB::CalculateSurfaceArea() const
	{
		return 2.0f * (GetWidth() * GetHeight() + GetWidth() * GetDepth() + GetHeight() * GetDepth());;
	}

	int AABB::CollideWithRay(const Vector3D& origin, const Vector3D& direction, std::vector<AABBHit>& hits) const
	{
		double m_ClipTemp[2];

		Vector3 extent = GetExtent();
		glm::vec3 diff = origin - (Vector3D)GetOrigin();

		m_ClipTemp[0] = 0.0;
		m_ClipTemp[1] = std::numeric_limits<double>::infinity();

		double saveT0 = m_ClipTemp[0];
		double saveT1 = m_ClipTemp[1];

		bool notEntirelyClipped =
				Clip(+direction.x, -diff.x - extent.x, m_ClipTemp) &&
				Clip(-direction.x, +diff.x - extent.x, m_ClipTemp) &&

				Clip(+direction.y, -diff.y - extent.y, m_ClipTemp) &&
				Clip(-direction.y, +diff.y - extent.y, m_ClipTemp) &&

				Clip(+direction.z, -diff.z - extent.z, m_ClipTemp) &&
				Clip(-direction.z, +diff.z - extent.z, m_ClipTemp);
		if (notEntirelyClipped && (m_ClipTemp[0] != saveT0 || m_ClipTemp[1] != saveT1))
		{
			if (m_ClipTemp[1] > m_ClipTemp[0])
			{
				glm::dvec3 point0 = (direction * m_ClipTemp[0]) + origin;
				glm::dvec3 point1 = (direction * m_ClipTemp[1]) + origin;

				hits.emplace_back(AABBHit{point0, m_ClipTemp[0]});
				hits.emplace_back(AABBHit{point1, m_ClipTemp[1]});
				return 2;
			} else {
				glm::dvec3 point = (direction * m_ClipTemp[0]) + origin;
				hits.emplace_back(AABBHit{point, m_ClipTemp[0]});
				return 1;
			}
		}
		return 0;
	}

	Vector3 AABB::GetRayHitNormal(const Vector3 hitPoint) const
	{
		Vector3 localPosition = hitPoint - GetOrigin();
		Vector3 extent = GetExtent();
		Vector3 normal;

		for (int axis = 0; axis < 3; ++axis)
		{
			if (glm::abs(glm::abs(localPosition[axis]) - extent[axis]) < glm::epsilon<float>())
			{
				normal[axis] = glm::sign(localPosition[axis]);
			}
			else
			{
				normal[axis] = 0;
			}
		}

		return normal;
	}

	bool AABB::Clip(double denom, double numer, double *t)
	{
		if (denom > 0.0)
		{
			double newT = numer / denom;
			if (newT > t[1])
			{
				return false;
			}
			if (newT > t[0])
			{
				t[0] = newT;
			}
			return true;
		}
		else if (denom < 0.0)
		{
			double newT = numer / denom;
			if (newT < t[0])
			{
				return false;
			}
			if (newT < t[1])
			{
				t[1] = newT;
			}
			return true;
		}
		else
		{
			return numer <= 0.0;
		}
	}

	BBCollisionResult AABB::CollideWithOther(const AABB& other)
	{
		Vector3 distances1 = other.m_Min - this->m_Max;
		Vector3 distances2 = this->m_Min - other.m_Max;
		Vector3 distances = glm::max(distances1, distances2);

		BBCollisionResult result;
		result.Distance = distances;

		for(int axis = 0; axis < 3; axis++) {
			result.CollidingAxes[axis] = distances[axis] < 0;
			result.CollidingAxesV[axis] = distances[axis] < 0;
			result.AxesDistances[axis] = distances[axis];
		}

		return result;
	}
}