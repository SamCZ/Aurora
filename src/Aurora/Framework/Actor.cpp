#include "Actor.hpp"
#include "Scene.hpp"

namespace Aurora
{

	void Actor::DestroyComponent(SceneComponent *&component)
	{
		if (component)
		{
			component->BeginDestroy();

			if (/*m_RootComponent != component && */m_Scene)
			{
				m_Scene->UnregisterComponent(component);
			}

			auto iter = std::find(m_Components.begin(), m_Components.end(), component);

			if(iter != m_Components.end()) {
				m_Components.erase(std::find(m_Components.begin(), m_Components.end(), component));
			}

			delete component;
			component = nullptr;
		}
	}

	void Actor::Destroy()
	{
		if (m_Scene)
		{
			m_Scene->DestroyActor(this);
		}
	}

	void Actor::InitializeComponent(SceneComponent *component)
	{
		component->m_Scene = this->m_Scene;
		component->m_Owner = this;

		if(m_Scene) {
			m_Scene->RegisterComponent(component);
		}
	}

	void Actor::AddForce(const Vector3 &force)
	{
		if(m_RootComponent == nullptr)
			return;

		m_RootComponent->m_Acceleration += force;
	}
}