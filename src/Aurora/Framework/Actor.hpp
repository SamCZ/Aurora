#pragma once

#include <type_traits>

#include "Object.hpp"
#include "Components/Base/SceneComponent.hpp"
#include "ComponentConcept.hpp"


namespace Aurora
{
	class Scene;
	class Material;

	class Actor : public Object
	{
		friend class ActorComponent;
		friend class SceneComponent;
		friend class Scene;
	private:
		bool m_IsActive;
		SceneComponent* m_RootComponent;
		std::vector<SceneComponent*> m_Components;
		Scene* m_Scene;
	public:
		inline Actor() : Object(), m_IsActive(false), m_RootComponent(nullptr), m_Components(), m_Scene(nullptr)
		{

		}

		~Actor() override = default;
	public:
		template<class T, typename... Args, BASE_OF(T, SceneComponent)>
		inline T* AddComponent(const String& name, Args&& ... args)
		{
			T* componentRaw = new T(std::forward<Args>(args)...);

			auto* component = reinterpret_cast<SceneComponent*>(componentRaw);

			component->Name = name;

			InitializeComponent(component);

			if (m_RootComponent != nullptr)
			{
				component->AttachToComponent(m_RootComponent);
			} else {
				component->m_Owner = this;
				component->m_Scene = m_Scene;
			}

			component->SetActive(true);

			return componentRaw;
		}

		inline void SetRootComponent(SceneComponent* component)
		{
			component->DetachFromComponent();
			DestroyComponent(m_RootComponent);

			component->m_Owner = this;
			m_RootComponent = component;
		}

		inline SceneComponent* GetRootComponent()
		{
			return m_RootComponent;
		}

		template<ComponentType T>
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

		inline virtual void InitializeComponents() {}
		inline virtual void BeginPlay() {}
		inline virtual void BeginDestroy() {}
		inline virtual void Tick(double delta) {}
		inline virtual void SetActive(bool newActive) { m_IsActive = newActive; }
		inline virtual void ToggleActive() { m_IsActive = !m_IsActive; }
		virtual inline bool IsActive() { return m_IsActive; }

		inline Scene* GetScene() { return m_Scene; }

		inline virtual void OnPreRender(Material* material) {}
	public:
		void DestroyComponent(SceneComponent*& component);
		virtual void Destroy();
	private:
		void InitializeComponent(SceneComponent* component);
	public:
		[[nodiscard]] inline const Vector3D& GetLocation() { return m_RootComponent->GetLocation(); }
		[[nodiscard]] inline const Vector3D& GetRotation() { return m_RootComponent->GetRotation(); }
		[[nodiscard]] inline const Vector3D& GetScale() { return m_RootComponent->GetScale(); }

		inline void SetLocation(const Vector3D& location) { m_RootComponent->SetLocation(location); }
		inline void SetRotation(const Vector3D& rotation) { m_RootComponent->SetRotation(rotation); }
		inline void SetScale(const Vector3D& scale) { m_RootComponent->SetScale(scale); }

		inline void SetLocation(double x, double y, double z) { m_RootComponent->SetLocation(x, y, z); }
		inline void SetRotation(double x, double y, double z) { m_RootComponent->SetRotation(x, y, z); }
		inline void SetScale(double x, double y, double z) { m_RootComponent->SetScale(x, y, z); }

		inline void AddLocation(const Vector3D& location) { m_RootComponent->AddLocation(location); }
		inline void AddRotation(const Vector3D& rotation) { m_RootComponent->AddRotation(rotation); }
		inline void AddScale(const Vector3D& scale) { m_RootComponent->AddScale(scale); }

		inline void AddLocation(double x, double y, double z) { m_RootComponent->AddLocation(x, y, z); }
		inline void AddRotation(double x, double y, double z) { m_RootComponent->AddRotation(x, y, z); }
		inline void AddScale(double x, double y, double z) { m_RootComponent->AddScale(x, y, z); }

		[[nodiscard]] inline Matrix4 GetTransformMatrix() { return m_RootComponent->GetTransformMatrix(); }

		[[nodiscard]] inline virtual Vector3D GetForwardVector() const { return m_RootComponent->GetForwardVector(); }
		[[nodiscard]] inline virtual Vector3D GetUpVector() const { return m_RootComponent->GetUpVector(); }
		[[nodiscard]] inline virtual Vector3D GetLeftVector() const { return m_RootComponent->GetLeftVector(); }
	private:
		void AddForce(const Vector3& force);
	};
}