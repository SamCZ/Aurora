#include "Transform.hpp"

namespace Aurora
{
	void Transform::UpdateRotationFromEuler()
	{
		glm::quat qYaw = glm::angleAxis(glm::radians(EulerRotation.x), glm::vec3(1, 0, 0));
		glm::quat qPitch = glm::angleAxis(glm::radians(EulerRotation.y), glm::vec3(0, 1, 0));
		glm::quat qRoll = glm::angleAxis(glm::radians(EulerRotation.z), glm::vec3(0, 0, 1));
		Rotation = glm::normalize(qPitch * qYaw * qRoll);
		MarkForUpdate();
	}

	void Transform::RotateAngleAxis(float angle, const Vector3& axis)
	{
		glm::quat qPitch = glm::angleAxis(glm::radians(angle), axis);
		Rotation = glm::normalize(qPitch * Rotation);
		MarkForUpdate();
	}

	void Transform::Rotate(const Vector3& angles, Transform::Space relativeTo)
	{
		glm::quat qYaw = glm::angleAxis(glm::radians(angles.x), glm::vec3(1, 0, 0));
		glm::quat qPitch = glm::angleAxis(glm::radians(angles.y), glm::vec3(0, 1, 0));
		glm::quat qRoll = glm::angleAxis(glm::radians(angles.z), glm::vec3(0, 0, 1));
		Quaternion euler = glm::normalize(qRoll * qYaw * qPitch);

		if(relativeTo == Space::Local)
		{
			Rotation = Rotation * euler;
		}
		else
		{
			Rotation = Rotation * (glm::inverse(Rotation) * euler * Rotation);
		}

		MarkForUpdate();
	}

	Matrix4 Transform::GetTransform() const
	{
		if (!NeedsUpdateMatrix)
		{
			return TransformMatrix;
		}

		// This is not very nice, but I cannot see any other solution rn...
		auto* thisTransform = const_cast<Transform*>(this);
		thisTransform->NeedsUpdateMatrix = false;

		return (thisTransform->TransformMatrix = glm::translate(glm::mat4(1.0f), Location) * glm::toMat4(Rotation) * glm::scale(glm::mat4(1.0f), Scale));
	}

	Matrix4 Transform::GetTransformNoScale() const
	{
		return glm::translate(glm::mat4(1.0f), Location) * glm::toMat4(Rotation);
	}

	void Transform::SetFromMatrix(const Matrix4& mat)
	{
		Vector3 rotation;
		glm::DecomposeTransform(mat, Location, rotation, Scale);
		Rotation = glm::quat_cast(mat);
		EulerRotation = glm::degrees(glm::eulerAngles(Rotation));
		TransformMatrix = mat;
		NeedsUpdateMatrix = false;
	}

	void Transform::SetFromMatrixNoScale(const Matrix4& mat)
	{
		Vector3 rotation;
		Vector3 scale;
		glm::DecomposeTransform(mat, Location, rotation, scale);
		Rotation = glm::quat_cast(mat);
		EulerRotation = glm::degrees(glm::eulerAngles(Rotation));
		TransformMatrix = mat;
		NeedsUpdateMatrix = false;
	}
}