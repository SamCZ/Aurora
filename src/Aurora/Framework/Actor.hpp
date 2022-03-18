#pragma once

#include <Aurora/Core/Library.hpp>
#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Object.hpp>
#include "SceneComponent.hpp"
#include "ComponentStorage.hpp"

#define DEFAULT_COMPONENT(name) typedef name DefaultComponent_t

namespace Aurora
{
	class Scene;

	class AU_API Actor : public ObjectBase
	{
		friend class Scene;
	protected:
		String m_Name;
		Scene* m_Scene;
		bool m_IsActive;
		SceneComponent* m_RootComponent;
		std::vector<SceneComponent*> m_Components;
	public:
		CLASS_OBJ(Actor, ObjectBase);
		DEFAULT_COMPONENT(SceneComponent);
	public:
		Actor();
		~Actor() override;

		template<typename T, typename... Args, typename std::enable_if<std::is_base_of<SceneComponent, T>::value>::type* = nullptr>
		T* AddComponent(const String& name, Args&& ... args)
		{
			SceneComponent* component = GetComponentStorage().CreateComponent<T, Args...>(name, std::forward<Args>(args)...);

			if(m_RootComponent)
			{
				component->AttachToComponent(m_RootComponent);
			}

			InitializeComponent(component);

			return (T*) component;
		}

		inline void SetRootComponent(SceneComponent* component)
		{
			component->DetachFromComponent();
			DestroyComponent(m_RootComponent);

			component->m_Owner = this;
			m_RootComponent = component;
		}

		inline SceneComponent* GetRootComponent() { return m_RootComponent; }

		template<class T, typename std::enable_if<std::is_base_of<SceneComponent, T>::value>::type* = nullptr>
		inline std::vector<T*> FindComponentsOfType()
		{
			std::vector<T*> components;

			if(T::TypeID() == m_RootComponent->GetTypeID()) {
				components.push_back(m_RootComponent);
			}

			// FIXME: This will only work in depth 1, sub components will not be found !

			for(auto* component : m_RootComponent->m_Components) {
				if(T::TypeID() == component->GetTypeID()) {
					components.push_back(component);
				}
			}

			return components;
		}

		template<class T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		inline T* FindComponentOfType()
		{
			// FIXME: This will only work in depth 1, sub components will not be found !

			if(T::TypeID() == m_RootComponent->GetTypeID()) {
				return (T*) m_RootComponent;
			}

			for(auto* component : m_RootComponent->m_Components) {
				if(T::TypeID() == component->GetTypeID()) {
					return (T*) component;
				}
			}
		}

		std::vector<SceneComponent*>::iterator begin() { return m_Components.begin(); }
		std::vector<SceneComponent*>::iterator end() { return m_Components.end(); }

		[[nodiscard]] std::vector<SceneComponent*>::const_iterator begin() const { return m_Components.begin(); }
		[[nodiscard]] std::vector<SceneComponent*>::const_iterator end() const { return m_Components.end(); }

		inline virtual void InitializeComponents() {}
		inline virtual void BeginPlay() {}
		inline virtual void BeginDestroy() {}
		inline virtual void Tick(double delta) {}
		inline virtual void SetActive(bool newActive) { m_IsActive = newActive; }
		inline virtual void ToggleActive() { m_IsActive = !m_IsActive; }
		virtual inline bool IsActive() { return m_IsActive; }

		[[nodiscard]] inline const String& GetName() const { return m_Name; }
		inline void SetName(const String& name) { m_Name = name; }

		inline Scene* GetScene() { return m_Scene; }

		void DestroyComponent(SceneComponent*& component);
		virtual void Destroy();

		const Transform& GetTransform() const;
		Transform& GetTransform();
	private:
		void InitializeComponent(SceneComponent* component);
		ComponentStorage& GetComponentStorage();
	};
}