#pragma once

#include "SceneRenderer.hpp"
#include "../Graphics/ViewPortManager.hpp"

namespace Aurora
{
	class AU_API SceneRendererForward : public SceneRenderer
	{
	private:
		Buffer_ptr m_VSDecalBuffer;
		Buffer_ptr m_PSDecalBuffer;

		InputLayout_ptr m_ParticleInputLayout;
		Shader_ptr m_ParticleComputeShader;
		Shader_ptr m_TonemappingShader;
		Shader_ptr m_BloomShader;
		Shader_ptr m_ScreenTextureShader;
		Shader_ptr m_ParticleRenderShader;
		Buffer_ptr m_BloomDescBuffer;

	public:
		SceneRendererForward();

		void LoadShaders() override;

		void Render(Scene* scene) override;
		TemporalRenderTarget BloomPass(RenderViewPort* viewPort, Texture_ptr texture, int pass, int operation, Texture_ptr biggerTexture);
		TemporalRenderTarget RenderBloom(RenderViewPort* viewPort);
	};
}
