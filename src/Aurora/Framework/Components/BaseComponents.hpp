#pragma once

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/UUID.hpp"
#include "Aurora/Graphics/Mesh.hpp"

// Some components were acquired from Hazel(Cherno game engine)

namespace Aurora
{
	class Material;

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent& other) = default;
		TagComponent(const std::string& tag)
				: Tag(tag) {}

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct RelationshipComponent
	{
		UUID ParentHandle = 0;
		std::vector<UUID> Children;

		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent& other) = default;
		RelationshipComponent(UUID parent) : ParentHandle(parent) {}
	};

	struct TransformComponent
	{
		Vector3 Translation = { 0.0f, 0.0f, 0.0f };
		Vector3 Rotation = { 0.0f, 0.0f, 0.0f };
		Vector3 Scale = { 1.0f, 1.0f, 1.0f };

		glm::vec3 Up = { 0.0f, 1.0f, 0.0f };
		glm::vec3 Right = { 1.0f, 0.0f, 0.0f };
		glm::vec3 Forward = { 0.0f, 0.0f, -1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;
		explicit TransformComponent(const glm::vec3& translation) : Translation(translation) {}

		[[nodiscard]] Matrix4 GetTransform() const
		{
			return  glm::translate(glm::mat4(1.0f), Translation)
					* glm::toMat4(glm::quat(Rotation))
					* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct CameraComponent
	{
		uint ID = 0;
		Vector2ui Size = {0, 0};
		float ZNear = 0.1f;
		float ZFar = 1000.f;
		float Fov = 85.0f;
		Matrix4 Projection = glm::identity<Matrix4>();

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other) = default;

		void SetupPerspectiveProjection()
		{
			Projection = glm::perspective(glm::radians(Fov), (float)Size.x / (float)Size.y, ZNear, ZFar);
		}

		void SetupOrthographicProjection(float left, float right, float bottom, float top)
		{
			Projection = glm::ortho(left, right, bottom, top);
		}
	};

	struct MeshComponent
	{
		std::shared_ptr<XMesh> Mesh;
		std::shared_ptr<MaterialSet> MaterialOverrides;

		MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;
	};
}