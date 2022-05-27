#pragma once

#include "SceneRenderer.hpp"

namespace Aurora
{
	class AU_API SceneRendererForward : public SceneRenderer
	{
	private:
		Buffer_ptr m_VSDecalBuffer;
		Buffer_ptr m_PSDecalBuffer;
	public:
		SceneRendererForward();

		void Render(Scene* scene) override;
	};
}
