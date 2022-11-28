#pragma once

#include "Actor.hpp"
#include "SceneComponent.hpp"

namespace Aurora
{
	class DecalComponent : public SceneComponent
	{
	public:
		CLASS_OBJ(DecalComponent, SceneComponent);
	};

	class Decal : public Actor
	{
	public:
		CLASS_OBJ(Decal, Actor);
		DEFAULT_COMPONENT(DecalComponent);
	};
}