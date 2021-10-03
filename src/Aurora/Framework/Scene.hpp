#pragma once

#include <entt/entt.hpp>

#include "Aurora/Core/String.hpp"
#include "Aurora/Memory/Aum.hpp"
#include "Actor.hpp"
#include "Components/BaseComponents.hpp"

namespace Aurora
{
	class Entity;

	class Scene
	{
	private:
		friend class Entity;
	private:
		entt::registry m_Registry;
		Aum m_ActorMemory;
		std::vector<Actor*> m_Actors;
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name);

		void Tick(double delta);

		Matrix4 GetTransformRelativeToParent(Entity entity);

		inline entt::registry& GetRegistry() { return m_Registry; }

		template<class Actor>
		Actor* SpawnActor(const String& name, const Vector3& position)
		{
			Actor* actor = m_ActorMemory.AllocAndInit<Actor>();
			actor->m_Scene = this;
			actor->Init();

			m_Actors.push_back(actor);

			return actor;
		}

		void DestroyActor(Actor* actor);

		inline Aum* GetActorMemory()
		{
			return &m_ActorMemory;
		}
	};
}