#pragma once

#include "SceneRenderer.hpp"

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
		Shader_ptr m_ScreenTextureShader;
		Shader_ptr m_ParticleRenderShader;

	public:
		SceneRendererForward();

		void Render(Scene* scene) override;
	};
}
