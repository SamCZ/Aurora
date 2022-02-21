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

		actor->SetActive(true);
		actor->BeginPlay();
	}

	void Scene::DestroyActor(Actor* actor)
	{
		if(!actor)
		{
			return;
		}

		for(SceneComponent* component : actor->m_Components)
		{
			actor->DestroyComponent(component);
		}

		actor->BeginDestroy();

		VectorRemove<Actor*>(m_Actors, actor);

		m_ActorMemory.DeAllocAndUnload<Actor>(actor);
	}
}