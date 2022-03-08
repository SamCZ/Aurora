#pragma once

#include "ActorComponent.hpp"
#include "Transform.hpp"

namespace Aurora
{
	class SceneComponent : public ActorComponent
	{
	private:
		Transform m_Transform;
		SceneComponent* m_Parent;
		std::vector<SceneComponent*> m_Components;
	public:
		friend class Actor;

		CLASS_OBJ(SceneComponent, ActorComponent);

		Transform& GetTransform() { return m_Transform; }

		[[nodiscard]] const Vector3& GetLocation() const { return m_Transform.Location; }
		[[nodiscard]] const Vector3& GetRotation() const { return m_Transform.Rotation; }
		[[nodiscard]] const Vector3& GetScale() const { return m_Transform.Scale; }

		[[nodiscard]] Matrix4 GetTransformationMatrix() const;

		bool AttachToComponent(SceneComponent* InParent);
		void DetachFromComponent();

		[[nodiscard]] const std::vector<SceneComponent*>& GetComponents() const { return m_Components; }
	};
}
