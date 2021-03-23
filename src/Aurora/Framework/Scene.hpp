#pragma once

#include "Actor.hpp"

#include "Components/Base/SceneComponent.hpp"
#include "Components/Mesh/MeshComponent.hpp"
#include "ComponentList.hpp"
#include "GameModeBase.hpp"

#define COMPONENT_LIST(name) \
ComponentList<name> m_##name##s;      \
public:                            \
[[nodiscard]] inline ComponentList<name>& Get##name##List() { return m_##name##s; } \
[[nodiscard]] inline const std::vector<name *>& Get##name##s() const { return m_##name##s.GetComponents(); } \

namespace Aurora
{
	AU_CLASS(Scene)
	{
		friend class Actor;
	private:
		std::vector<Actor*> m_Actors;

		COMPONENT_LIST(MeshComponent)
		COMPONENT_LIST(SceneComponent)

		GameModeBase* m_GameMode;
	public:
		inline Scene() : m_Actors(), m_MeshComponents(), m_GameMode(nullptr) { }

		inline ~Scene()
		{
			auto actors = m_Actors;

			for(auto* actor : actors) {
				actor->Destroy();
			}

			if(m_GameMode != nullptr) {
				m_GameMode->BeginDestroy();
			}
			delete m_GameMode;
		}

		template<class T, BASE_OF(T, Actor)>
		T* SpawnActor(const String& Name, const Vector3D& Position, const Vector3D& Rotation = Vector3D(0.0), const Vector3D& Scale = Vector3D(1.0))
		{
			T* actor = BeginSpawnActor<T>(Name, Position, Rotation, Scale);

			FinishSpawningActor(actor);

			return actor;
		}

		template<class T, BASE_OF(T, Actor)>
		T* BeginSpawnActor(const String& Name, const Vector3D& Position, const Vector3D& Rotation, const Vector3D& Scale = Vector3D(1.0))
		{
			T* actorTemplated = new T();
			Actor* actor = static_cast<Actor*>(actorTemplated);
			actor->m_Scene = this;
			actor->Name = Name;

			if (!actor->m_RootComponent)
			{
				actor->m_RootComponent = actor->AddComponent<SceneComponent>("SceneRoot");
			}

			actor->InitializeComponents();

			actor->SetLocation(Position);
			actor->SetRotation(Rotation);
			actor->SetScale(Scale);

			m_Actors.push_back(actor);

			return actorTemplated;
		}

		inline void FinishSpawningActor(Actor* actor)
		{
			if(!actor) {
				return;
			}

			actor->SetActive(true);
			actor->BeginPlay();
		}

		inline void DestroyActor(Actor* actor)
		{
			if(actor) {
				// Destroy components of actor

				for (auto* component : actor->m_Components) {
					component->BeginDestroy();
					UnregisterComponent(component);
				}

				actor->m_Components.clear();

				delete actor->m_RootComponent;
				actor->m_RootComponent = nullptr;

				// Destroy actor
				actor->BeginDestroy();

				auto iter = std::find(m_Actors.begin(), m_Actors.end(), actor);
				if (iter != m_Actors.end())
				{
					m_Actors.erase(iter);
					delete actor;
				}
			}
		}

		inline void RegisterComponent(ActorComponent* component)
		{
			if(auto* cmp = component->SafeCast<MeshComponent>()) {
				m_MeshComponents.Add(cmp);
			}

			if(auto* cmp = component->SafeCast<SceneComponent>()) {
				m_SceneComponents.Add(cmp);
			}
		}

		inline void UnregisterComponent(ActorComponent* component)
		{
			if(auto* cmp = component->SafeCast<MeshComponent>()) {
				m_MeshComponents.Remove(cmp);
			}

			if(auto* cmp = component->SafeCast<SceneComponent>()) {
				m_SceneComponents.Remove(cmp);
			}
		}

		[[nodiscard]] inline const std::vector<Actor*>& GetActors() const
		{
			return m_Actors;
		}
	public:
		template<typename T, typename... Args, BASE_OF(T, GameModeBase)>
		inline T* OverrideGameMode(Args&&... args)
		{
			if(m_GameMode != nullptr) {
				m_GameMode->BeginDestroy();
				delete m_GameMode;
			}

			T* newGamemode = new T(std::forward<Args>(args)...);
			newGamemode->m_Scene = this;
			newGamemode->BeginPlay();

			m_GameMode = newGamemode;
			return newGamemode;
		}
	public:
		inline void Update(double delta)
		{
			if(m_GameMode != nullptr) {
				m_GameMode->Tick(delta);
			}

			m_SceneComponents.PreUpdateSystems(delta);
			m_MeshComponents.PreUpdateSystems(delta);

			for(Actor* actor : m_Actors) {
				actor->Tick(delta);
			}

			m_SceneComponents.UpdateSystems(delta);
			m_MeshComponents.UpdateSystems(delta);

			m_SceneComponents.UpdateComponents(delta);

			m_SceneComponents.PostUpdateSystems(delta);
			m_MeshComponents.PostUpdateSystems(delta);
		}
	};
}
#undef COMPONENT_LIST