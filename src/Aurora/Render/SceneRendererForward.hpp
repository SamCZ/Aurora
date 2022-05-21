#pragma once

#include "SceneRenderer.hpp"

namespace Aurora
{
	class AU_API SceneRendererForward : public SceneRenderer
	{
	public:
		SceneRendererForward() : SceneRenderer() {}

		void Render(Scene* scene) override;
	};
}
