#pragma once

#include "SceneComponent.hpp"

namespace Aurora
{
	class PrimitiveComponent : public SceneComponent
	{
	public:
		inline PrimitiveComponent() : SceneComponent() {}
		~PrimitiveComponent() override = default;
	};
}