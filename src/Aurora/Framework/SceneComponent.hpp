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

		bool AttachToComponent(SceneComponent* InParent);
		void DetachFromComponent();
	};
}
