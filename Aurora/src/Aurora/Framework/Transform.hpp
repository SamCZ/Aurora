#pragma once

#include "Aurora/Core/Math.hpp"

namespace Aurora
{
	class AU_API Transform
	{
	public:
		enum class Space
		{
			Local, World
		};
	private:
		Vector3 Location = { 0.0f, 0.0f, 0.0f };
		Quaternion Rotation = glm::identity<Quaternion>();
		Vector3 EulerRotation = { 0.0f, 0.0f, 0.0f };
		Vector3 Scale = { 1.0f, 1.0f, 1.0f };
		mutable Matrix4 TransformMatrix = glm::identity<Matrix4>();
		mutable bool NeedsUpdateMatrix = true;
	public:
		Transform() = default;
		Transform(const Transform& other) = default;
		explicit Transform(const glm::vec3& location) : Location(location) {}

		void MarkForUpdate() { NeedsUpdateMatrix = true; }

		void UpdateRotationFromEuler();

		void SetLocation(const Vector3& location) { Location = location; MarkForUpdate(); }
		void AddLocation(const Vector3& location) { Location += location; MarkForUpdate(); }
		void SetLocation(float x, float y, float z) { Location.x = x; Location.y = y; Location.z = z; MarkForUpdate(); }
		void AddLocation(float x, float y, float z) { Location.x += x; Location.y += y; Location.z += z; MarkForUpdate(); }
		[[nodiscard]] const Vector3& GetLocation() const { return Location; }

		void SetRotation(const Vector3& eulerAngles) { EulerRotation = eulerAngles; UpdateRotationFromEuler(); }
		void AddRotation(const Vector3& eulerAngles) { EulerRotation += eulerAngles; UpdateRotationFromEuler(); }
		void SetRotation(float x, float y, float z) { EulerRotation.x = x; EulerRotation.y = y; EulerRotation.z = z; UpdateRotationFromEuler(); }
		void AddRotation(float x, float y, float z) { EulerRotation.x += x; EulerRotation.y += y; EulerRotation.z += z; UpdateRotationFromEuler(); }
		[[nodiscard]] const Vector3& GetRotation() const { return EulerRotation; }

		void SetRotation(const Quaternion& quaternion) { Rotation = quaternion; MarkForUpdate(); }
		void AddRotation(const Quaternion& quaternion) { Rotation *= quaternion; MarkForUpdate(); }
		[[nodiscard]] const Quaternion& GetRotationQuaternion() const { return Rotation; }

		void SetScale(const Vector3& scale) { Scale = scale; MarkForUpdate(); }
		void AddScale(const Vector3& scale) { Scale += scale; MarkForUpdate(); }
		void SetScale(float x, float y, float z) { Scale.x = x; Scale.y = y; Scale.z = z; MarkForUpdate(); }
		void SetScale(float xyz) { Scale.x = xyz; Scale.y = xyz; Scale.z = xyz; MarkForUpdate(); }
		void AddScale(float x, float y, float z) { Scale.x += x; Scale.y += y; Scale.z += z; MarkForUpdate(); }
		[[nodiscard]] const Vector3& GetScale() const { return Scale; }

		void RotateAngleAxis(float angle, const Vector3& axis);
		void Rotate(const Vector3& angles, Space relativeTo);

		[[nodiscard]] Matrix4 GetTransform() const;
		[[nodiscard]] Matrix4 GetTransformNoScale() const;

		void SetFromMatrix(const Matrix4& mat);
		void SetFromMatrixNoScale(const Matrix4& mat);

		[[nodiscard]] Vector3D GetForwardVector() const { return GetTransform()[2]; }
		[[nodiscard]] Vector3D GetUpVector() const { return GetTransform()[1]; }
		[[nodiscard]] Vector3D GetLeftVector() const { return GetTransform()[0]; }
	};
}