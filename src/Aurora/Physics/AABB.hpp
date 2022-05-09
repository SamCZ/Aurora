#pragma once

#include <Aurora/Core/Vector.hpp>

namespace Aurora
{
	struct BBCollisionResult
	{
		Vector3 Distance;
		Vector3i CollidingAxesV;
		bool CollidingAxes[3]{};
		double AxesDistances[3]{};
	};

	class AU_API AABB
	{
	private:
		Vector3 m_Min;
		Vector3 m_Max;
		float m_SurfaceArea;
	public:
		AABB();
		AABB(const Vector3& min, const Vector3& max);
		~AABB();

		static AABB FromExtent(const Vector3& origin, const Vector3& extent);

		[[nodiscard]] const Vector3& GetMin() const;
		[[nodiscard]] const Vector3& GetMax() const;

		void SetOffset(const Vector3& offset);
		void SetOffset(double x, double y, double z);

		void Set(const Vector3& min, const Vector3& max);

		void Extend(const Vector3& point);

		[[nodiscard]] bool IntersectsWith(const AABB& other) const;

		[[nodiscard]] bool Overlaps(const AABB& other) const
		{
			// y is deliberately first in the list of checks below as it is seen as more likely than things
			// collide on x,z but not on y than they do on y thus we drop out sooner on a y fail
			return m_Max.x > other.m_Min.x &&
				m_Min.x < other.m_Max.x &&
				m_Max.y > other.m_Min.y &&
				m_Min.y < other.m_Max.y &&
				m_Max.z > other.m_Min.z &&
				m_Min.z < other.m_Max.z;
		}

		[[nodiscard]] bool Contains(const AABB& other) const
		{
			return other.m_Min.x >= m_Min.x &&
				other.m_Max.x <= m_Max.x &&
				other.m_Min.y >= m_Min.y &&
				other.m_Max.y <= m_Max.y &&
				other.m_Min.z >= m_Min.z &&
				other.m_Max.z <= m_Max.z;
		}

		[[nodiscard]] AABB Merge(const AABB& other) const
		{
			return {
				{std::min(m_Min.x, other.m_Min.x), std::min(m_Min.y, other.m_Min.y), std::min(m_Min.z, other.m_Min.z)},
				{std::max(m_Max.x, other.m_Max.x), std::max(m_Max.y, other.m_Max.y), std::max(m_Max.z, other.m_Max.z)}
			};
		}

		[[nodiscard]] AABB Intersection(const AABB& other) const
		{
			return {
				{std::max(m_Min.x, other.m_Min.x), std::max(m_Min.y, other.m_Min.y), std::max(m_Min.z, other.m_Min.z)},
				{std::min(m_Max.x, other.m_Max.x), std::min(m_Max.y, other.m_Max.y), std::min(m_Max.z, other.m_Max.z)}
			};
		}

		[[nodiscard]] Vector3 GetOrigin() const;
		[[nodiscard]] Vector3 GetExtent() const;
		[[nodiscard]] Vector3 GetSize() const { return m_Max - m_Min; }

		[[nodiscard]] inline float GetWidth() const { return m_Max.x - m_Min.x; }
		[[nodiscard]] inline float GetHeight() const { return m_Max.y - m_Min.y; }
		[[nodiscard]] inline float GetDepth() const { return m_Max.z - m_Min.z; }
		[[nodiscard]] inline float GetSurfaceArea() const { return m_SurfaceArea; }

		[[nodiscard]] float CalculateSurfaceArea() const;

		//int CollideWithRay(const Ray& ray, CollisionResults& results) const override;

		static void CheckMinMax(Vector3& min, Vector3& max, const Vector3& point);

		BBCollisionResult CollideWithOther(const AABB& other);

		friend std::ostream& operator<<(std::ostream& os, const AABB& bounds)
		{
			os << "BoundingBox(min=" << glm::to_string(bounds.GetMin()) << ",max=" << glm::to_string(bounds.GetMax()) << ")";
			return os;
		}

		[[nodiscard]] AABB Transform(const Matrix4& matrix) const
		{
			Vector3 corners[8];

			corners[0] = m_Min;
			corners[1] = Vector3(m_Min.x, m_Min.y, m_Max.z);
			corners[2] = Vector3(m_Min.x, m_Max.y, m_Min.z);
			corners[3] = Vector3(m_Max.x, m_Min.y, m_Min.z);
			corners[4] = Vector3(m_Min.x, m_Max.y, m_Max.z);
			corners[5] = Vector3(m_Max.x, m_Min.y, m_Max.z);
			corners[6] = Vector3(m_Max.x, m_Max.y, m_Min.z);
			corners[7] = m_Max;

			auto min = Vector3(std::numeric_limits<float>::max());
			auto max = Vector3(std::numeric_limits<float>::min());

			for(auto & corner : corners) {
				Vector4 transformed = matrix * Vector4(corner, 1.0);
				min = glm::min(min, Vector3(transformed));
				max = glm::max(max, Vector3(transformed));
			}

			return {min, max};
		}

		AABB& operator*=(const Matrix4& matrix)
		{
			*this = Transform(matrix);
			return *this;
		}

		AABB operator*(const Matrix4& matrix)
		{
			return Transform(matrix);
		}
	private:
		static bool Clip(double denom, double numer, double t[]);
	};
}