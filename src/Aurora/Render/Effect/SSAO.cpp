#include "SSAO.hpp"

namespace Aurora
{

	bool SSAOEffect::CanRender() const
	{
		return PostProcessEffect::CanRender();
	}

	void SSAOEffect::Init()
	{

	}

	void SSAOEffect::Render(const Texture_ptr &input, const Texture_ptr &output)
	{
		/*static float ssaoRadius = 3.0f;
		static float ssaoBias = 0.025f;

		ImGui::Begin("SSAO");
		{
			ImGui::DragFloat("Bias", &ssaoBias, 0.01f);
			ImGui::DragFloat("Radius", &ssaoRadius, 0.01f);
		}
		ImGui::End();

		auto ssaoRT = m_RenderManager->CreateTemporalRenderTarget("SSAO", camera.Size, GraphicsFormat::SRGBA8_UNORM);
		if(false) { // SSAO
			DrawCallState drawState;
			drawState.Shader = m_SSAOShader;
			drawState.PrimitiveType = EPrimitiveType::TriangleStrip;
			drawState.ClearDepthTarget = false;
			drawState.ClearColorTarget = true;
			drawState.RasterState.CullMode = ECullMode::None;
			drawState.DepthStencilState.DepthEnable = false;
			drawState.ViewPort = camera.Size;

			drawState.BindTarget(0, ssaoRT);
			drawState.BindTexture("WorldPositionRT", worldPosRT);
			drawState.BindTexture("NormalWorldRT", normalsRT);
			drawState.BindTexture("NoiseTex", ssaoNoiseTex);

			std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
			std::default_random_engine generator;
			std::vector<glm::vec4> ssaoKernel;
			for (unsigned int i = 0; i < SSAO_SAMPLE_COUNT; ++i)
			{
				glm::vec4 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator), 1);
				sample = glm::normalize(sample);
				sample *= randomFloats(generator);
				float scale = float(i) / float(SSAO_SAMPLE_COUNT);

				// scale samples s.t. they're more aligned to center of kernel
				scale = lerp(0.1f, 1.0f, scale * scale);
				sample *= scale;
				ssaoKernel.push_back(sample);
			}

			BEGIN_UB(SSAODesc, desc)
				desc->ProjectionMatrix = projectionMatrix;
				desc->ViewMatrix = viewMatrix;
				desc->NoiseData = vec4((Vector2)camera.Size / 4.0f, ssaoRadius, ssaoBias);
				memcpy(desc->Samples, ssaoKernel.data(), ssaoKernel.size() * sizeof(Vector4));
			END_UB(SSAODesc);

			m_RenderDevice->Draw(drawState, {DrawArguments(4)});
			m_RenderManager->GetUniformBufferCache().Reset();
		}*/
	}
}