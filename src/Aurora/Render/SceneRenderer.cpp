#include "SceneRenderer.hpp"

#include "Aurora/Framework/Scene.hpp"

namespace Aurora
{
	SceneRenderer::SceneRenderer(Scene *scene) : m_Scene(scene)
	{

	}

	SceneRenderer::~SceneRenderer()
	{

	}

	void SceneRenderer::Render()
	{
		assert(m_Scene);

		auto meshesView = m_Scene->GetRegistry().view<TransformComponent, MeshComponent>();

		for(entt::entity meshesEntt : meshesView)
		{
			auto [transform, meshComponent] = meshesView.get<TransformComponent, MeshComponent>(meshesEntt);
			const std::shared_ptr<XMesh>& mesh = meshComponent.Mesh;

			if(mesh == nullptr) continue;


		}
	}
}