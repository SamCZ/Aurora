#include "Actor.hpp"
#include "Scene.hpp"

namespace Aurora
{
	Actor::Actor() : m_Scene(nullptr), m_EntityHandle(entt::null)
	{

	}

	Actor::~Actor()
	{

	}

	void Actor::Destroy()
	{
		m_Scene->DestroyActor(this);
	}
}