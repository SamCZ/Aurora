#pragma once

#include "Scene.hpp"

namespace Aurora
{
	class Entity
	{
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene)
				: m_EntityHandle(handle), m_Scene(scene) {}

		~Entity() = default;

		operator entt::entity() const
		{
			return m_EntityHandle;
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			//assert(!HasComponent<T>(), "Entity already has component!");
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			//assert(HasComponent<T>(), "Entity doesn't have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		const T& GetComponent() const
		{
			//assert(HasComponent<T>(), "Entity doesn't have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
		}

		template<typename T>
		[[nodiscard]] bool HasComponent() const
		{
			return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
		}

		template<typename...T>
		bool HasAny()
		{
			return m_Scene->m_Registry.any_of<T...>(m_EntityHandle);
		}

		template<typename...T>
		[[nodiscard]] bool HasAny() const
		{
			return m_Scene->m_Registry.any_of<T...>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			HZ_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		TransformComponent& Transform() { return m_Scene->m_Registry.get<TransformComponent>(m_EntityHandle); }
		[[nodiscard]] Matrix4 GetModelMatrix() const { return m_Scene->m_Registry.get<TransformComponent>(m_EntityHandle).GetTransform(); }
	};
}