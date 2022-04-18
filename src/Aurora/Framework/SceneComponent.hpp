#pragma once

#include "ActorComponent.hpp"
#include "Transform.hpp"

namespace Aurora
{
	class AU_API SceneComponent : public ActorComponent
	{
	private:
		Transform m_Transform;
		SceneComponent* m_Parent;
		std::vector<SceneComponent*> m_Components;
	public:
		friend class Actor;

		CLASS_OBJ(SceneComponent, ActorComponent);

		SceneComponent();
		~SceneComponent() override = default;

		[[nodiscard]] const Transform& GetTransform() const { return m_Transform; }
		Transform& GetTransform() { return m_Transform; }

		[[nodiscard]] const Vector3& GetLocation() const { return m_Transform.Location; }
		[[nodiscard]] const Vector3& GetRotation() const { return m_Transform.Rotation; }
		[[nodiscard]] const Vector3& GetScale() const { return m_Transform.Scale; }

		[[nodiscard]] Matrix4 GetTransformationMatrix() const;
		[[nodiscard]] Vector3 GetWorldPosition() const { return GetTransformationMatrix()[3]; }
		[[nodiscard]] Vector3 GetForwardVector() const { return GetTransformationMatrix()[2]; }
		[[nodiscard]] Vector3 GetUpVector() const { return GetTransformationMatrix()[1]; }
		[[nodiscard]] Vector3 GetLeftVector() const { return GetTransformationMatrix()[0]; }

		bool AttachToComponent(SceneComponent* InParent);
		void DetachFromComponent();

		[[nodiscard]] const std::vector<SceneComponent*>& GetComponents() const { return m_Components; }

		SceneComponent* GetParent() { return m_Parent; }
		[[nodiscard]] bool HasParent() const { return m_Parent != nullptr;}
		[[nodiscard]] bool IsParentActive() const;
	};
}
