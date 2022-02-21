#pragma once

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Logger/Logger.hpp"
#include "Aurora/Memory/Aum.hpp"
#include "ComponentStorage.hpp"
#include "Actor.hpp"
#include "SceneComponent.hpp"

namespace Aurora
{

	class Scene
	{
	private:
		Aum m_ActorMemory;
		std::vector<Actor*> m_Actors;
		ComponentStorage m_ComponentStorage;
	public:
		friend class Actor;

		Scene();
		~Scene();

		template<class T, class RootCmp = SceneComponent, typename std::enable_if<std::is_base_of<Actor, T>::value>::type* = nullptr>
		T* SpawnActor(const String& name, const Vector3& position, const Vector3& rotation = Vector3(0.0), const Vector3& scale = Vector3(1.0))
		{
			T* actor = BeginSpawnActor<T, RootCmp>(name, position, rotation, scale);
			FinishSpawningActor(actor);
			return actor;
		}

		template<class T, class RootCmp = SceneComponent, typename std::enable_if<std::is_base_of<Actor, T>::value>::type* = nullptr>
		T* BeginSpawnActor(const String& name, const Vector3& position, const Vector3& rotation = Vector3(0.0), const Vector3& scale = Vector3(1.0))
		{
			size_t objSize = sizeof(T);
			size_t objSizeAligned = Align(objSize, 16u);

			MemPtr actorMemory = m_ActorMemory.Alloc(objSizeAligned);
			Actor* actor = new(actorMemory) T();

			actor->m_Scene = this;
			actor->m_Name = name;

			actor->m_RootComponent = m_ComponentStorage.CreateComponent<RootCmp>("RootComponent");
			actor->InitializeComponent(actor->m_RootComponent);

			actor->m_RootComponent->GetTransform().Translation = position;
			actor->m_RootComponent->GetTransform().Rotation = rotation;
			actor->m_RootComponent->GetTransform().Scale = scale;

			actor->InitializeComponents();

			m_Actors.push_back(actor);

			return (T*) actor;
		}

		/*template<typename... T>
		const ComponentView GetComponents()
		{

		}*/

		std::vector<Actor*>::iterator begin() { return m_Actors.begin(); }
		std::vector<Actor*>::iterator end() { return m_Actors.end(); }

		[[nodiscard]] std::vector<Actor*>::const_iterator begin() const { return m_Actors.begin(); }
		[[nodiscard]] std::vector<Actor*>::const_iterator end() const { return m_Actors.end(); }

	protected:


	public:
		void FinishSpawningActor(Actor* actor);
		void DestroyActor(Actor* actor);
	};
}
