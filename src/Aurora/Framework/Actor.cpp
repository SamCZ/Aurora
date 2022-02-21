#include "Actor.hpp"
#include "Scene.hpp"
#include "Aurora/Core/Common.hpp"

namespace Aurora
{
	Actor::Actor() : m_Scene(nullptr), m_IsActive(false), m_RootComponent(nullptr)
	{

	}

	Actor::~Actor() = default;

	void Actor::Destroy()
	{
		if (m_Scene)
		{
			m_Scene->DestroyActor(this);
		}
	}

	void Actor::DestroyComponent(SceneComponent*& component)
	{
		if(!component)
		{
			return;
		}

		component->BeginDestroy();

		if(m_RootComponent != component && m_Scene)
		{
			//m_Scene->UnregisterComponent(component);
		}

		VectorRemove<SceneComponent*>(m_Components, std::forward<SceneComponent*>(component));
		GetComponentStorage().DestroyComponent(component);
		component = nullptr;
	}

	void Actor::InitializeComponent(SceneComponent* component)
	{
		component->m_Scene = this->m_Scene;
		component->m_Owner = this;

		if(m_Scene)
		{
			//m_Scene->RegisterComponent(component);
		}

		m_Components.push_back(component);
	}

	ComponentStorage& Actor::GetComponentStorage()
	{
		return m_Scene->m_ComponentStorage;
	}
}