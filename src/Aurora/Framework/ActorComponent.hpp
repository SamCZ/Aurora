#pragma once

#include "Aurora/Core/Object.hpp"
#include "Aurora/Core/String.hpp"

namespace Aurora
{
	class Actor;
	class Scene;

	class SceneComponent;

	class AU_API ActorComponent : public ObjectBase
	{
	protected:
		String m_Name;
		bool m_IsActive;
		Actor* m_Owner;
		Scene* m_Scene;
	protected:
		SceneComponent* m_Parent;
	public:
		friend class Actor;
		CLASS_OBJ(ActorComponent, ObjectBase);
	public:
		ActorComponent() : ObjectBase(), m_IsActive(false), m_Owner(nullptr), m_Scene(nullptr), m_Parent(nullptr) { }
		~ActorComponent() override = default;

		virtual inline void SetActive(bool newActive) { m_IsActive = newActive; }
		virtual inline void ToggleActive() { m_IsActive = !m_IsActive; }
		[[nodiscard]] virtual inline bool IsActive() const { return m_IsActive; }
		virtual inline void Tick(double delta) { }
		virtual inline void BeginPlay() {}
		virtual inline void BeginDestroy() {}
		[[nodiscard]] virtual Actor* GetOwner() const { return m_Owner; }

		void SetName(const String& name) { m_Name = name; }
		[[nodiscard]] const String& GetName() const { return m_Name; }

		SceneComponent* GetParent() const { return m_Parent; }
		[[nodiscard]] bool HasParent() const { return m_Parent != nullptr;}
		[[nodiscard]] bool IsParentActive() const;

		bool AttachToComponent(SceneComponent* InParent);
		void DetachFromComponent();
	};
}
