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

	void Actor::DestroyComponent(ActorComponent*& component)
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

		VectorRemove<ActorComponent*>(m_Components, component);
		GetComponentStorage().DestroyComponent(component);
		component = nullptr;
	}

	void Actor::InitializeComponent(ActorComponent* component)
	{
		component->m_Scene = this->m_Scene;
		component->m_Owner = this;

		if(m_Scene)
		{
			//m_Scene->RegisterComponent(component);
		}

		m_Components.push_back(component);

		component->SetActive(true);
		component->BeginPlay();
	}

	ComponentStorage& Actor::GetComponentStorage()
	{
		return m_Scene->m_ComponentStorage;
	}

	const Transform& Actor::GetTransform() const
	{
		return m_RootComponent->GetTransform();
	}

	Transform& Actor::GetTransform()
	{
		return m_RootComponent->GetTransform();
	}
}