#pragma once

#include <Aurora/Core/Library.hpp>
#include <Aurora/Core/String.hpp>
#include <Aurora/Core/Object.hpp>
#include "SceneComponent.hpp"
#include "ComponentStorage.hpp"

#define DEFAULT_COMPONENT(name) typedef name DefaultComponent_t

namespace Aurora
{
	class AU_API Scene;

	class AU_API Actor : public ObjectBase
	{
		friend class Scene;
	protected:
		String m_Name;
		Scene* m_Scene;
		bool m_IsActive;
		SceneComponent* m_RootComponent;
		std::vector<ActorComponent*> m_Components;
	public:
		CLASS_OBJ(Actor, ObjectBase);
		DEFAULT_COMPONENT(SceneComponent);
	public:
		Actor();
		~Actor() override;

		template<typename T, typename... Args, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		T* AddComponent(Args&& ... args)
		{
			ActorComponent* component = GetComponentStorage().CreateComponent<T, Args...>(T::TypeName(), std::forward<Args>(args)...);

			if(m_RootComponent)
			{
				component->AttachToComponent(m_RootComponent);
			}

			InitializeComponent(component);

			return (T*) component;
		}

		template<typename T, typename... Args, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		T* AddComponentNamed(const String& name, Args&& ... args)
		{
			ActorComponent* component = GetComponentStorage().CreateComponent<T, Args...>(name, std::forward<Args>(args)...);

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
			DestroyComponent((ActorComponent*&)m_RootComponent);

			component->m_Owner = this;
			m_RootComponent = component;
		}

		inline SceneComponent* GetRootComponent() { return m_RootComponent; }

		template<class T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		inline T* GetRootComponent()
		{
			return T::SafeCast(GetRootComponent());
		}

		template<class T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		inline std::vector<T*> FindComponentsOfType()
		{
			std::vector<T*> components;

			for (ActorComponent* component : m_Components)
			{
				if(component->HasType(T::TypeID()))
				{
					components.push_back(T::Cast(component));
				}
			}

			return components;
		}

		template<class T, typename std::enable_if<std::is_base_of<ActorComponent, T>::value>::type* = nullptr>
		inline T* FindComponentOfType()
		{
			for (ActorComponent* component : m_Components)
			{
				if(component->HasType(T::TypeID()))
				{
					return T::Cast(component);
				}
			}

			return nullptr;
		}

		std::vector<ActorComponent*>::iterator begin() { return m_Components.begin(); }
		std::vector<ActorComponent*>::iterator end() { return m_Components.end(); }

		[[nodiscard]] std::vector<ActorComponent*>::const_iterator begin() const { return m_Components.begin(); }
		[[nodiscard]] std::vector<ActorComponent*>::const_iterator end() const { return m_Components.end(); }

		inline virtual void InitializeComponents() {}
		inline virtual void BeginPlay() {}
		inline virtual void BeginDestroy() {}
		inline virtual void Tick(double delta) {}
		inline virtual void FixedStep() {}
		inline virtual void SetActive(bool newActive) { m_IsActive = newActive; }
		inline virtual void ToggleActive() { m_IsActive = !m_IsActive; }
		virtual inline bool IsActive() { return m_IsActive; }

		[[nodiscard]] inline const String& GetName() const { return m_Name; }
		inline void SetName(const String& name) { m_Name = name; }

		inline Scene* GetScene() { return m_Scene; }

		void DestroyComponent(ActorComponent*& component);
		virtual void Destroy();

		[[nodiscard]] const Transform& GetTransform() const;
		Transform& GetTransform();
	private:
		void InitializeComponent(ActorComponent* component);
		ComponentStorage& GetComponentStorage();
	};
}