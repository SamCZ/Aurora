#pragma once

#include "ActorComponent.hpp"
#include "Transform.hpp"

namespace Aurora
{
	class AU_API SceneComponent : public ActorComponent
	{
	private:
		Transform m_Transform;
		std::vector<ActorComponent*> m_Components;
	public:
		friend class Actor;
		friend class ActorComponent;

		CLASS_OBJ(SceneComponent, ActorComponent);

		SceneComponent();
		~SceneComponent() override = default;

		[[nodiscard]] const Transform& GetTransform() const { return m_Transform; }
		Transform& GetTransform() { return m_Transform; }

		[[nodiscard]] const Vector3& GetLocation() const { return m_Transform.GetLocation(); }
		[[nodiscard]] const Vector3& GetRotation() const { return m_Transform.GetRotation(); }
		[[nodiscard]] const Vector3& GetScale() const { return m_Transform.GetScale(); }

		[[nodiscard]] Matrix4 GetTransformationMatrix() const;
		[[nodiscard]] Vector3 GetWorldPosition() const { return GetTransformationMatrix()[3]; }
		[[nodiscard]] Vector3 GetForwardVector() const { return GetTransformationMatrix()[2]; }
		[[nodiscard]] Vector3 GetUpVector() const { return GetTransformationMatrix()[1]; }
		[[nodiscard]] Vector3 GetLeftVector() const { return GetTransformationMatrix()[0]; }

		[[nodiscard]] const std::vector<ActorComponent*>& GetComponents() const { return m_Components; }

		template<typename T>
		void GetComponentsOfType(std::vector<T*>& components)
		{
			if(HasType(T::TypeID()))
			{
				components.push_back(T::Cast(this));
			}

			for(ActorComponent* component : m_Components)
			{
				if (SceneComponent* sceneComponent = SceneComponent::SafeCast(component))
				{
					sceneComponent->GetComponentsOfType<T>(components);
				}
			}
		}
	};
}
