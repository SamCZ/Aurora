#pragma once

#include <entt/entt.hpp>

#include "Components/BaseComponents.hpp"

namespace Aurora
{
	class Entity;

	class Scene
	{
	private:
		friend class Entity;
	private:
		entt::registry m_Registry;
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name);

		void Tick(double delta);

		Matrix4 GetTransformRelativeToParent(Entity entity);

		inline entt::registry& GetRegistry() { return m_Registry; }
	};
}