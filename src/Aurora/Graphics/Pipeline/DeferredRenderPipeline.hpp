#pragma once

#include "../RenderPipeline.hpp"
#include "../SceneRenderer.hpp"

namespace Aurora
{
	class Scene;

	class DeferredRenderPipeline : public RenderPipeline
	{
	private:
		SceneRenderer m_SceneRenderer;
	public:
		explicit DeferredRenderPipeline(std::shared_ptr<Scene> scene);
		~DeferredRenderPipeline() override;

		void Update(double delta) override;
		void Render() override;
		Texture_ptr GetFinalRT(uint camera) override;
		Texture_ptr GetIntermediateRT(const std::string& name) override;
	};
}