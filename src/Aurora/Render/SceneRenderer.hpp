#pragma once

namespace Aurora
{
	class Scene;

	class SceneRenderer
	{
	private:
		Scene* m_Scene;
	public:
		explicit SceneRenderer(Scene* scene);
		~SceneRenderer();

		//void RenderPass();

		void Render();
	};
}
