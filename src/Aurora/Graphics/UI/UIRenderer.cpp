#include "UIRenderer.hpp"

namespace Aurora
{
	/*struct UIUniform
	{
		float scissorMat[12]; // matrices are actually 3 vec4s
		float paintMat[12];
		Vector4 innerCol;
		Vector4 outerCol;
		float scissorExt[2];
		float scissorScale[2];
		float extent[2];
		float radius;
		float feather;
		float strokeMult;
		float strokeThr;
		int texType;
		int type;
	};*/

	struct UIVertexUniform
	{
		Matrix4 Projection;
		Matrix4 ModelMat;
	};

	struct UIFragmentUniform
	{
		uint32_t Type;
		float StrokeSize;
		float Radius;
		float c;
		Vector4 Color;
		Vector2 Size;
		Vector2 d;
	};

	static float depth = 0;

	UIRenderer::UIRenderer() : m_Material(nullptr)
	{
		m_Material = std::make_shared<Material>("UI", "Assets/Shaders/UI");

		m_Material->SetCullMode(CULL_MODE_NONE);
		m_Material->SetDepthEnable(true);
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
		/*MapHelper<UIVertexUniform> vertexUniform;
		if(m_Material->GetConstantBuffer("VertexUniform", vertexUniform)) {
			vertexUniform->Projection = m_ProjectionMatrix;
			vertexUniform->ModelMat = glm::translate(Vector3(x, y, -0.5 + (depth++) * 0.1)) * glm::scale(Vector3(w, h, 1));
		}

		MapHelper<UIFragmentUniform> fragmentUniform;
		if(m_Material->GetConstantBuffer("FragmentUniform", fragmentUniform)) {
			fragmentUniform->Type = 0;
			fragmentUniform->StrokeSize = 0;
			fragmentUniform->Radius = radius;
			fragmentUniform->Color = color;
			fragmentUniform->Size.x = w;
			fragmentUniform->Size.y = h;
		}*/

		m_Material->CommitShaderResources();

		DrawAttribs drawAttrs;
		drawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
		drawAttrs.NumVertices = 4;
		AuroraEngine::ImmediateContext->Draw(drawAttrs);
	}

	void UIRenderer::DrawRect(float x, float y, float w, float h, const Vector4& color, float strokeSize, float radius)
	{
		/*MapHelper<UIVertexUniform> vertexUniform;
		if(m_Material->GetConstantBuffer("VertexUniform", vertexUniform)) {
			vertexUniform->Projection = m_ProjectionMatrix;
			vertexUniform->ModelMat = glm::translate(Vector3(x, y, -0.5 + (depth++) * 0.1)) * glm::scale(Vector3(w, h, 1));
		}

		MapHelper<UIFragmentUniform> fragmentUniform;
		if(m_Material->GetConstantBuffer("FragmentUniform", fragmentUniform)) {
			fragmentUniform->Type = 1;
			fragmentUniform->StrokeSize = strokeSize;
			fragmentUniform->Radius = radius;
			fragmentUniform->Color = color;
			fragmentUniform->Size.x = w;
			fragmentUniform->Size.y = h;
		}*/

		m_Material->CommitShaderResources();

		DrawAttribs drawAttrs;
		drawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
		drawAttrs.NumVertices = 4;
		AuroraEngine::ImmediateContext->Draw(drawAttrs);
	}

	void UIRenderer::DrawImage(float x, float y, float w, float h, float radius)
	{

	}
}