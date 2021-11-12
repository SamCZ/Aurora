#pragma once

#include <Aurora/Core/Vector.hpp>

namespace Aurora
{
	struct BBCollisionResult
	{
		bool CollidingAxes[3]{};
		double AxesDistances[3]{};
	};

	class AABB
	{
	private:
		Vector3 m_Min;
		Vector3 m_Max;
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

		[[nodiscard]] Vector3 GetOrigin() const;
		[[nodiscard]] Vector3 GetExtent() const;

		//int CollideWithRay(const Ray& ray, CollisionResults& results) const override;

		static void CheckMinMax(Vector3& min, Vector3& max, const Vector3& point);

		BBCollisionResult CollideWithOther(const AABB& other);

		friend std::ostream& operator<<(std::ostream& os, const AABB& bounds)
		{
			os << "BoundingBox(min=" << glm::to_string(bounds.GetMin()) << ",max=" << glm::to_string(bounds.GetMax()) << ")";
			return os;
		}

		AABB& operator*=(const Matrix4& matrix)
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

			Vector3 min = Vector3(1) * std::numeric_limits<float>::infinity();
			Vector3 max = Vector3(1) * -std::numeric_limits<float>::infinity();

			for(auto & corner : corners) {
				Vector4 transformed = matrix * Vector4(corner, 1.0);
				min = glm::min(min, Vector3(transformed));
				max = glm::max(max, Vector3(transformed));
			}

			Set(min, max);

			return *this;
		}
	private:
		static bool Clip(double denom, double numer, double t[]);
	};
}