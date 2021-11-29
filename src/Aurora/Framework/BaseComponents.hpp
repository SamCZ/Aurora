#pragma once

#include <utility>

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/UUID.hpp"
#include "Aurora/Graphics/Mesh.hpp"

// Some components were acquired from Hazel(Cherno game engine)

class ndBodyDynamic;

namespace Aurora
{
	class Material;
	AU_CLASS(PostProcessEffect);

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent& other) = default;
		explicit TagComponent(std::string tag) : Tag(std::move(tag)) {}

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct RelationshipComponent
	{
		UUID64 ParentHandle = 0;
		std::vector<UUID64> Children;

		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent& other) = default;
		explicit RelationshipComponent(const UUID64& parent) : ParentHandle(parent) {}
	};

	struct TransformComponent
	{
		Vector3 Translation = { 0.0f, 0.0f, 0.0f };
		Vector3 Rotation = { 0.0f, 0.0f, 0.0f };
		Vector3 Scale = { 1.0f, 1.0f, 1.0f };

		glm::vec3 Up = { 0.0f, 1.0f, 0.0f };
		glm::vec3 Right = { 1.0f, 0.0f, 0.0f };
		glm::vec3 Forward = { 0.0f, 0.0f, -1.0f };

		bool Locked = false; // For debug purposes

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;
		explicit TransformComponent(const glm::vec3& translation) : Translation(translation) {}

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

	struct CameraComponent
	{
		uint ID = 0;
		Vector2ui Size = {0, 0};
		float ZNear = 0.1f;
		float ZFar = 1000.f;
		float Fov = 85.0f;
		Matrix4 Projection = glm::identity<Matrix4>();
		std::vector<PostProcessEffect_ptr> PostProcessEffects;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other) = default;

		void SetupPerspectiveProjection()
		{
			Projection = glm::perspective(glm::radians(Fov), (float)Size.x / (float)Size.y, ZNear, ZFar);
		}

		void SetupOrthographicProjection(float left, float right, float bottom, float top, float near, float far)
		{
			//Projection = glm::ortho(left, right, bottom, top, near, far);
		}

		[[nodiscard]] Matrix4 GetProjectionViewMatrix(const TransformComponent& transformComponent) const
		{
			return Projection * glm::inverse(transformComponent.GetTransform());
		}

		[[nodiscard]] Vector3 GetWorldPosition(const TransformComponent& transformComponent, double x, double y, double projectionZPos) const
		{
			Matrix4 inverseMat = glm::inverse(GetProjectionViewMatrix(transformComponent));

			Vector3 store = Vector3((2.0 * x) / (float)Size.x - 1.0, (2.0 * y) / (float)Size.y - 1.0, projectionZPos * 2 - 1);
			Vector4 proStore = inverseMat * Vector4(store, 1.0);
			store.x = proStore.x;
			store.y = proStore.y;
			store.z = proStore.z;
			store *= 1.0f / proStore.w;
			return store;
		}

		bool GetScreenCoordinates(const TransformComponent& transformComponent, const Vector3D& position, Vector2& out_ScreenPos) const
		{
			Vector4 result = (GetProjectionViewMatrix(transformComponent) * Vector4(position, 1.0f));

			if(result.w > 0.0f) {
				// the result of this will be x and y coords in -1..1 projection space
				const float RHW = 1.0f / result.w;
				Vector4 PosInScreenSpace = Vector4(result.x * RHW, result.y * RHW, result.z * RHW, result.w);

				// Move from projection space to normalized 0..1 UI space
				const float NormalizedX = ( PosInScreenSpace.x / 2.f ) + 0.5f;
				const float NormalizedY = 1.f - ( PosInScreenSpace.y / 2.f ) - 0.5f;

				out_ScreenPos.x = NormalizedX * (float)Size.x;
				out_ScreenPos.y = NormalizedY * (float)Size.y;

				if(out_ScreenPos.x < 0 || out_ScreenPos.x > (float)Size.x) {
					return false;
				}

				if(out_ScreenPos.y < 0 || out_ScreenPos.y > (float)Size.y) {
					return false;
				}

				return true;
			}

			return false;
		}
	};

	struct MeshComponent
	{
		std::shared_ptr<XMesh> Mesh;
		std::shared_ptr<MaterialSet> MaterialOverrides = std::make_shared<MaterialSet>();

		MeshComponent() = default;
		~MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;

		std::shared_ptr<Material> GetMaterial(int materialIndex)
		{
			if(Mesh == nullptr) return nullptr;

			if(MaterialOverrides != nullptr && MaterialOverrides->HasMaterial(materialIndex))
			{
				return MaterialOverrides->GetMaterial(materialIndex);
			}

			if(Mesh->Materials != nullptr && Mesh->Materials->HasMaterial(materialIndex))
			{
				return Mesh->Materials->GetMaterial(materialIndex);
			}

			return nullptr;
		}
	};

	struct BodyComponent
	{
		ndBodyDynamic* Body;
	};

	struct BaseColliderComponent
	{
		Vector3 Offset;
	};

	struct BoxColliderComponent : BaseColliderComponent
	{
		AABB BoundingBox;
	};

	struct SphereColliderComponent : BaseColliderComponent
	{
		float Radius;
	};

	struct MeshColliderComponent : BaseColliderComponent
	{
		int test;
		// TODO
	};

	enum class EShadowMode : uint8_t
	{
		None = 0,
		Hard,
		Soft
	};

	struct LightComponent
	{
		float Intensity = 1.0f;
		Color LightColor = Color::white();
		EShadowMode ShadowMode = EShadowMode::Soft;
		float Near = 1.0f;
		float Far  = 500.0f;
	};

	struct DirectionalLightComponent : LightComponent
	{

	};

	struct PointLightComponent : LightComponent
	{
		// TODO
	};

	struct SpotLightComponent : LightComponent
	{
		// TODO
	};
}