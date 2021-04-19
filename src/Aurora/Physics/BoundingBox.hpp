#pragma once

#include <Aurora/Core/Vector.hpp>

#include "Bound.hpp"

namespace Aurora
{
	struct BBCollisionResult
	{
		bool CollidingAxes[3]{};
		double AxesDistances[3]{};
	};

	class BoundingBox : public Bound
	{
	private:
		Vector3D m_Min;
		Vector3D m_Max;
	public:
		BoundingBox();
		BoundingBox(const Vector3D& min, const Vector3D& max);
		~BoundingBox() override;

		static BoundingBox FromExtent(const Vector3D& origin, const Vector3D& extent);

		[[nodiscard]] const Vector3D& GetMin() const;
		[[nodiscard]] const Vector3D& GetMax() const;

		void SetOffset(const Vector3D& offset);
		void SetOffset(double x, double y, double z);

		void Set(const Vector3D& min, const Vector3D& max);

		void Extend(const Vector3D& point);

		[[nodiscard]] bool IntersectsWith(const BoundingBox& other) const;

		[[nodiscard]] Vector3D GetOrigin() const;
		[[nodiscard]] Vector3D GetExtent() const;

		int CollideWithRay(const Ray& ray, CollisionResults& results) const override;

		static void CheckMinMax(Vector3D& min, Vector3D& max, const Vector3D& point);

		BBCollisionResult CollideWithOther(const BoundingBox& other);

		friend std::ostream& operator<<(std::ostream& os, const BoundingBox& bounds)
		{
			os << "BoundingBox(min=" << glm::to_string(bounds.GetMin()) << ",max=" << glm::to_string(bounds.GetMax()) << ")";
			return os;
		}

		BoundingBox& operator*=(const Matrix4& matrix)
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
