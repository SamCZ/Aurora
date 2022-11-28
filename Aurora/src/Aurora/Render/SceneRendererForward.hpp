#pragma once

#include "SceneRenderer.hpp"
#include "Aurora/Graphics/Material/MaterialDefinition.hpp"

namespace Aurora
{
	class CameraComponent;

	class AU_API SceneRendererForward : public SceneRenderer
	{
	private:
		Buffer_ptr m_VSDecalBuffer;
		Buffer_ptr m_PSDecalBuffer;

		InputLayout_ptr m_ParticleInputLayout;
		Shader_ptr m_ParticleComputeShader;
		Shader_ptr m_ParticleRenderShader;

		Shader_ptr m_FinalPostShader;
	public:
		SceneRendererForward();

		void LoadShaders() override;

		void Render(Scene* scene, CameraComponent* debugCamera) override;
	};
}
