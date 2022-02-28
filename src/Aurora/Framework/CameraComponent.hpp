#pragma once

#include "SceneComponent.hpp"

namespace Aurora
{
	class CameraComponent : public SceneComponent
	{
	private:
		int a;
	public:
		CLASS_OBJ(CameraComponent, SceneComponent);

		~CameraComponent() override = default;
	};
}
