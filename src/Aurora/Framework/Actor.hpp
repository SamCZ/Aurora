#pragma once

#include <Aurora/Core/Library.hpp>
#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Object.hpp>
#include "SceneComponent.hpp"
#include "ComponentStorage.hpp"

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
	public:
		Actor();
		~Actor() override;

		template<typename T, typename... Args, typename std::enable_if<std::is_base_of<SceneComponent, T>::value>::type* = nullptr>
		T* AddComponent(const String& name, Args&& ... args)
		{
			SceneComponent* component = GetComponentStorage().CreateComponent<T, Args...>(name, std::forward<Args>(args)...);

			InitializeComponent(component);

			if(m_RootComponent)
			{
				component->AttachToComponent(m_RootComponent);
			}

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

			if(auto* c = dynamic_cast<T*>(m_RootComponent)) {
				components.push_back(m_RootComponent);
			}

			for(auto* component : m_RootComponent->m_Components) {
				if(auto* c = dynamic_cast<T*>(component)) {
					components.push_back(component);
				}
			}

			return components;
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

		inline Scene* GetScene() { return m_Scene; }

		void DestroyComponent(SceneComponent*& component);
		virtual void Destroy();
	private:
		void InitializeComponent(SceneComponent* component);
		ComponentStorage& GetComponentStorage();
	};
}