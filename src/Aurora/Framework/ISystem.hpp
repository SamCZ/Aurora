#pragma once

#include <vector>
#include "ComponentConcept.hpp"

namespace Aurora
{
	template<ComponentType T>
	class ISystem
	{
	public:
		friend class ActorComponent;
		friend class SceneComponent;
		friend class PrimitiveComponent;
	public:
		virtual ~ISystem() = default;
	public:
		virtual void PreUpdate(const std::vector<T*>& components, double delta) {}
		virtual void Update(const std::vector<T*>& components, double delta) = 0;
		virtual void PostUpdate(const std::vector<T*>& components, double delta) {}
	};
}