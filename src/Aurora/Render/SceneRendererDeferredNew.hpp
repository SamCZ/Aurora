#pragma once

#include "SceneRenderer.hpp"

namespace Aurora
{
	class AU_API SceneRendererDeferredNew : public SceneRenderer
	{
	public:
		SceneRendererDeferredNew();

		void LoadShaders() override;
		void Render(Scene* scene) override;
	};
}
