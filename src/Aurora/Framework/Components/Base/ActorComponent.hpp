#pragma once

#include "../../Object.hpp"
#include "Aurora/Core/Vector.hpp"
#include "../../Layer.hpp"

namespace Aurora
{
	class Actor;
	class Scene;

	class ActorComponent : public Object
	{
		friend class Actor;
	protected:
		std::vector<String> m_Tags;
		Layer m_Layer;
		bool m_IsActive;
		Actor* m_Owner;
		Scene* m_Scene;
	public:
		inline ActorComponent() : Object(), m_Tags(), m_IsActive(false), m_Owner(nullptr), m_Scene(nullptr), m_Layer() { }

		~ActorComponent() override = default;
	public:
		virtual inline void SetActive(bool newActive) { m_IsActive = newActive; }
		virtual inline void ToggleActive() { m_IsActive = !m_IsActive; }
		virtual inline bool IsActive() { return m_IsActive; }
		virtual inline void Tick(double delta) { }
		virtual inline void BeginPlay() {}
		virtual inline void BeginDestroy() {}
		virtual Actor* GetOwner() { return m_Owner; }

		virtual inline const Matrix4& GetTransformMatrix() { static auto identity = glm::identity<Matrix4>(); return identity; }

		inline Layer& GetLayer() { return m_Layer; }
	};
}