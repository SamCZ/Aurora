#pragma once

#include <Aurora/Core/Common.hpp>
#include <Aurora/Framework/Scene.hpp>

#include "RenderTargetManager.hpp"

namespace Aurora
{
	AU_CLASS(SceneRenderer)
	{
	private:
		Scene_ptr m_Scene;
		std::map<CameraComponent*, std::map<Material*, std::vector<std::tuple<Mesh*, uint32_t, Matrix4, Actor*>>>> m_SortedRenderer;
	public:
		explicit SceneRenderer(Scene_ptr scene);

		void Update(double delta, Frustum* frustum);
		void Render(RenderTargetPack* renderTargetPack, bool apply, bool clear);
	};
}
