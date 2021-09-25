#include "AABB.hpp"

namespace Aurora
{
	AABB::AABB() : m_Min(), m_Max()
	{

	}

	AABB::AABB(const Vector3 &min, const Vector3 &max) : m_Min(min), m_Max(max)
	{

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
	}

	void AABB::Extend(const Vector3 &point)
	{
		CheckMinMax(m_Min, m_Max, point);
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
		return m_Min + ((m_Max - m_Min) / 2.0f);
	}

	Vector3 AABB::GetExtent() const
	{
		return (m_Max - m_Min) / 2.0f;
	}

	/*int AABB::CollideWithRay(const Ray &ray, CollisionResults &results) const
	{
		double m_ClipTemp[2];

		Vector3 extent = GetExtent();
		glm::vec3 diff = ray.Origin - GetOrigin();
		glm::vec3 direction = ray.Direction;

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
				glm::dvec3 point0 = (ray.Direction * m_ClipTemp[0]) + ray.Origin;
				glm::dvec3 point1 = (ray.Direction * m_ClipTemp[1]) + ray.Origin;

				CollisionResult result;
				result.ContactPoint = point0;
				result.Distance = m_ClipTemp[0];
				results.AddCollision(result);

				CollisionResult result2;
				result2.ContactPoint = point1;
				result2.Distance = m_ClipTemp[1];
				results.AddCollision(result2);
			} else {
				glm::dvec3 point = (ray.Direction * m_ClipTemp[0]) + ray.Origin;
				CollisionResult result;
				result.ContactPoint = point;
				result.Distance = m_ClipTemp[0];
				results.AddCollision(result);
			}
			return 1;
		}
		return 0;
	}*/

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
		Vector3D distances1 = other.m_Min - this->m_Max;
		Vector3D distances2 = this->m_Min - other.m_Max;
		Vector3D distances = glm::max(distances1, distances2);

		BBCollisionResult result;

		for(int axis = 0; axis < 3; axis++) {
			result.CollidingAxes[axis] = distances[axis] < 0;
			result.AxesDistances[axis] = distances[axis];
		}

		return result;
	}
}