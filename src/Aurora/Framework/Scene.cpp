#include "Scene.hpp"
#include "Aurora/Core/Common.hpp"

namespace Aurora
{

	Scene::Scene() : m_ActorMemory()
	{

	}

	Scene::~Scene()
	{
		std::vector<Actor*> actors = m_Actors;
		for(Actor* actor : actors)
		{
			DestroyActor(actor);
		}
	}

	void Scene::FinishSpawningActor(Actor* actor)
	{
		if(!actor) {
			return;
		}

		m_Actors.push_back(actor);

		actor->SetActive(true);
		actor->BeginPlay();
	}

	void Scene::DestroyActor(Actor* actor)
	{
		if(!actor)
		{
			return;
		}

		for (size_t i = actor->m_Components.size(); i --> 0;)
		{
			actor->DestroyComponent(actor->m_Components[i]);
		}

		actor->BeginDestroy();

		VectorRemove<Actor*>(m_Actors, actor);

		m_ActorMemory.DeAllocAndUnload<Actor>(actor);
	}

	void Scene::Update(double delta)
	{
		// Iterate from end to enable destroy while updating
		for (size_t i = m_Actors.size(); i --> 0;)
		{
			m_Actors[i]->Tick(delta);
		}

		for(ActorComponent* actorComponent : GetComponents<ActorComponent>())
		{
			actorComponent->Tick(delta);
		}
	}
}