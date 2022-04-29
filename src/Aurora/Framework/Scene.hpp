#pragma once

#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Logger/Logger.hpp"
#include "Aurora/Memory/Aum.hpp"
#include "ComponentStorage.hpp"
#include "SceneComponent.hpp"
#include "Actor.hpp"

#include <btBulletDynamicsCommon.h>

namespace Aurora
{
	class BulletDebugDraw;
	class btLayerOverlapFilterCallback;

	class AU_API Scene
	{
	private:
		Aum m_ActorMemory;
		std::vector<Actor*> m_Actors;
		ComponentStorage m_ComponentStorage;

		btDefaultCollisionConfiguration* m_CollisionConfiguration;
		btCollisionDispatcher* m_Dispatcher;
		btDbvtBroadphase* m_Broadphase;
		btSequentialImpulseConstraintSolver* m_Solver;
		btDiscreteDynamicsWorld* m_DynamicsWorld;
		BulletDebugDraw* m_DebugDraw;
		btLayerOverlapFilterCallback* m_OverlapCallback;
	public:
		friend class Actor;

		Scene();
		~Scene();

		btDynamicsWorld* GetDynamicsWorld() const { return m_DynamicsWorld; }

		template<class T, class RootCmp = typename T::DefaultComponent_t, typename std::enable_if<std::is_base_of<Actor, T>::value>::type* = nullptr>
		T* SpawnActor(const String& name, const Vector3& position, const Vector3& rotation = Vector3(0.0), const Vector3& scale = Vector3(1.0))
		{
			T* actor = BeginSpawnActor<T, RootCmp>(name, position, rotation, scale);
			FinishSpawningActor(actor);
			return actor;
		}

		template<class T, class RootCmp = SceneComponent, typename std::enable_if<std::is_base_of<Actor, T>::value>::type* = nullptr>
		T* BeginSpawnActor(const String& name, const Vector3& position, const Vector3& rotation = Vector3(0.0), const Vector3& scale = Vector3(1.0))
		{
			au_assert(name.empty() == false);

			size_t objSize = sizeof(T);
			size_t objSizeAligned = Align(objSize, 16u);

			MemPtr actorMemory = m_ActorMemory.Alloc(objSizeAligned);
			Actor* actor = new(actorMemory) T();

			actor->m_Scene = this;
			actor->m_Name = name;

			actor->m_RootComponent = m_ComponentStorage.CreateComponent<RootCmp>("RootComponent");
			actor->InitializeComponent(actor->m_RootComponent);

			actor->m_RootComponent->GetTransform().SetLocation(position);
			actor->m_RootComponent->GetTransform().SetRotation(rotation);
			actor->m_RootComponent->GetTransform().SetScale(scale);

			actor->InitializeComponents();

			return (T*) actor;
		}

		std::vector<Actor*>::iterator begin() { return m_Actors.begin(); }
		std::vector<Actor*>::iterator end() { return m_Actors.end(); }

		[[nodiscard]] std::vector<Actor*>::const_iterator begin() const { return m_Actors.begin(); }
		[[nodiscard]] std::vector<Actor*>::const_iterator end() const { return m_Actors.end(); }

		template<typename T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		ComponentView<T> GetComponents()
		{
			return m_ComponentStorage.template GetComponents<T>();
		}

		template<typename T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		T* FindFirstComponent()
		{
			return m_ComponentStorage.template FindFirstComponent<T>();
		}

		void Update(double delta);

	public:
		void FinishSpawningActor(Actor* actor);
		void DestroyActor(Actor* actor);

		void RegisterComponent(SceneComponent* component);
		void UnRegisterComponent(SceneComponent* component);
	};
}
