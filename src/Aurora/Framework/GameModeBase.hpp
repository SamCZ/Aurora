#pragma once

#include "Object.hpp"

namespace Aurora
{
	class GameModeBase : public Object
	{
	public:
		friend class Scene;
	protected:
		class Scene* m_Scene;
	public:
		inline GameModeBase() : m_Scene(nullptr)
		{

		}

		inline virtual void BeginPlay() {}
		inline virtual void BeginDestroy() {}
		inline virtual void Tick(double delta) {}
	};
}