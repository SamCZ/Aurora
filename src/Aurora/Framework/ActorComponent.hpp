#pragma once

#include "Aurora/Core/Object.hpp"
#include "Aurora/Core/String.hpp"

namespace Aurora
{
	class Actor;
	class Scene;

	class ActorComponent : public ObjectBase
	{
	protected:
		String m_Name;
		bool m_IsActive;
		Actor* m_Owner;
		Scene* m_Scene;
	public:
		friend class Actor;
		CLASS_OBJ(ActorComponent, ObjectBase);
	public:
		inline ActorComponent() : ObjectBase(), m_IsActive(false), m_Owner(nullptr), m_Scene(nullptr) { }
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
	};
}
