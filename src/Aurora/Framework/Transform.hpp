#pragma once

#include "Aurora/Core/Vector.hpp"

namespace Aurora
{
	struct Transform
	{
		Vector3 Translation = { 0.0f, 0.0f, 0.0f };
		Vector3 Rotation = { 0.0f, 0.0f, 0.0f };
		Vector3 Scale = { 1.0f, 1.0f, 1.0f };

		glm::vec3 Up = { 0.0f, 1.0f, 0.0f };
		glm::vec3 Right = { 1.0f, 0.0f, 0.0f };
		glm::vec3 Forward = { 0.0f, 0.0f, -1.0f };

		bool Locked = false; // For debug purposes

		Transform() = default;
		Transform(const Transform& other) = default;
		explicit Transform(const glm::vec3& translation) : Translation(translation) {}

		[[nodiscard]] Matrix4 GetTransform() const
		{
			const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f),
				glm::radians(Rotation.x),
				glm::vec3(1.0f, 0.0f, 0.0f));
			const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f),
				glm::radians(Rotation.y),
				glm::vec3(0.0f, 1.0f, 0.0f));
			const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f),
				glm::radians(Rotation.z),
				glm::vec3(0.0f, 0.0f, 1.0f));

			// Y * X * Z
			const glm::mat4 roationMatrix = transformY * transformX * transformZ;

			return  glm::translate(glm::mat4(1.0f), Translation) * roationMatrix * glm::scale(glm::mat4(1.0f), Scale);
		}

		void SetFromMatrix(const Matrix4& mat)
		{
			glm::DecomposeTransform(mat, Translation, Rotation, Scale);
			Rotation = glm::degrees(Rotation);
		}
	};
}