#pragma once

#include <entt/entt.hpp>

namespace Aurora
{
	class Scene;

	class Actor
	{
		friend class Scene;
	protected:
		Scene* m_Scene;
		entt::entity m_EntityHandle;
	public:
		Actor();
		virtual ~Actor();

		virtual void Init() {}
		virtual void Tick(double delta) {}

		void Destroy();

		inline void SetEntityHandle(entt::entity entityHandle) { m_EntityHandle = entityHandle; }

		inline entt::entity EntityHandle() { return m_EntityHandle; }
	};
}