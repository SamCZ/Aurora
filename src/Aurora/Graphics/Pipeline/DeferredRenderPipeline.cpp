#include "DeferredRenderPipeline.hpp"

namespace Aurora
{
	DeferredRenderPipeline::DeferredRenderPipeline(std::shared_ptr<Scene> scene) : m_SceneRenderer(scene)
	{

	}

	DeferredRenderPipeline::~DeferredRenderPipeline()
	{

	}

	void DeferredRenderPipeline::Update(double delta)
	{
		m_SceneRenderer.Update(delta);
	}

	void DeferredRenderPipeline::Render()
	{

	}

	Texture_ptr DeferredRenderPipeline::GetFinalRT(uint camera)
	{
		return Aurora::Texture_ptr();
	}

	Texture_ptr DeferredRenderPipeline::GetIntermediateRT(const std::string &name)
	{
		return Aurora::Texture_ptr();
	}
}