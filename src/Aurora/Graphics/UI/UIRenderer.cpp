#include "UIRenderer.hpp"

namespace Aurora
{
	static float depth = 0;

	UIRenderer::UIRenderer() : m_Material(nullptr), m_ProjectionMatrix()
	{
		m_Material = std::make_shared<Material>("UI", "Assets/Shaders/UI");

		m_Material->SetCullMode(CULL_MODE_NONE);
		m_Material->SetDepthEnable(false);
		m_Material->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

		RenderTargetBlendDesc blendDesc;
		blendDesc.BlendEnable = true;
		blendDesc.SrcBlend = BLEND_FACTOR_SRC_ALPHA;
		blendDesc.DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;

		m_Material->SetBlendState(blendDesc);
	}

	void UIRenderer::Begin(const Vector2i& size, const TEXTURE_FORMAT& textureFormat, const TEXTURE_FORMAT& depthFormat)
	{
		m_ProjectionMatrix = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -1.0f, 1.0f);

		GraphicsPipelineDesc& graphicsPipelineDesc = m_Material->GetPipelineDesc();
		graphicsPipelineDesc.NumRenderTargets = 1;
		graphicsPipelineDesc.RTVFormats[0] = textureFormat;
		graphicsPipelineDesc.DSVFormat = depthFormat;

		m_Material->ValidateGraphicsPipelineState();
		m_Material->ApplyPipeline();
		depth = 0;
	}

	void UIRenderer::End()
	{

	}

	void UIRenderer::FillRect(float x, float y, float w, float h, const Vector4 &color, float radius)
	{
		DrawArgs drawArgs;
		drawArgs.Color = color;
		drawArgs.Radius = radius;
		Draw(x, y, w, h, drawArgs);
	}

	void UIRenderer::DrawRect(float x, float y, float w, float h, const Vector4& color, float strokeSize, float radius)
	{
		DrawArgs drawArgs;
		drawArgs.Color = color;
		drawArgs.Fill = false;
		drawArgs.StrokeSize = strokeSize;
		drawArgs.Radius = radius;
		Draw(x, y, w, h, drawArgs);
	}

	void UIRenderer::Draw(float x, float y, float w, float h, const DrawArgs& drawArgs)
	{
		if(!drawArgs.Fill) {
			float strokeHalf = drawArgs.StrokeSize / 2.0f;
			x -= strokeHalf;
			y -= strokeHalf;
			w += strokeHalf * 2.0f;
			h += strokeHalf * 2.0f;
		}

		m_Material->SetVariable<Matrix4>("Projection", m_ProjectionMatrix);
		m_Material->SetVariable<Matrix4>("ModelMat", glm::translate(Vector3(x, y, 0)) * glm::scale(Vector3(w, h, 1)));

		if(drawArgs.Texture != nullptr) {
			m_Material->SetVariable<uint32_t>("Type", 0);
			m_Material->SetTexture("Texture", drawArgs.Texture);
			m_Material->SetVariable<Vector4>("Color", drawArgs.Tint);
		} else {
			if(drawArgs.Fill) {
				m_Material->SetVariable<uint32_t>("Type", 1);
			} else {
				m_Material->SetVariable<uint32_t>("Type", 2);
				m_Material->SetVariable<float>("StrokeSize", drawArgs.StrokeSize / 2.0f);
			}

			m_Material->SetVariable<Vector4>("Color", drawArgs.Color);
		}

		m_Material->SetVariable<Vector2>("Size", Vector2(w, h));
		m_Material->SetVariable<float>("Radius", drawArgs.Radius);

		//m_Material->SetVariable<uint32_t>("Type", 4);
		//m_Material->SetVariable<Vector4>("Color", color);

		m_Material->SetVariable<int>("ImageEdgeDetectionEnabled", false);
		m_Material->SetVariable<int>("EdgeThickness", 5);
		m_Material->SetVariable<Vector4>("EdgeColor", Vector4(1.0));

		m_Material->CommitShaderResources();

		DrawAttribs drawAttrs;
		drawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
		drawAttrs.NumVertices = 4;
		AuroraEngine::ImmediateContext->Draw(drawAttrs);
	}

	void UIRenderer::DrawImage(float x, float y, float w, float h, const RefCntAutoPtr<ITexture> &texture, float radius)
	{
		DrawArgs drawArgs;
		drawArgs.Texture = texture;
		drawArgs.Radius = radius;
		drawArgs.Fill = true;
		Draw(x, y, w, h, drawArgs);
	}
}