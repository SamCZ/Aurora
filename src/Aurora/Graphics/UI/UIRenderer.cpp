#include "UIRenderer.hpp"

namespace Aurora
{
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

		SetImageEdgeDetection(false, 3);
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

		if(drawArgs.EnabledCustomUVs) {
			static Vector4 cacheUvVal[4];

			for (int i = 0; i < 4; ++i) {
				cacheUvVal[i].x = drawArgs.CustomUVs[i].x;
				cacheUvVal[i].y = drawArgs.CustomUVs[i].y;
			}

			m_Material->SetArray("UVs", cacheUvVal, sizeof(Vector4) * 4);
			m_Material->SetVariable<int>("CustomUVsEnabled", true);
		} else {
			m_Material->SetVariable<int>("CustomUVsEnabled", false);
		}

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

		m_Material->CommitShaderResources();

		DrawAttribs drawAttrs;
		drawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
		drawAttrs.NumVertices = 4;
		AuroraEngine::ImmediateContext->Draw(drawAttrs);
	}

	void UIRenderer::DrawImage(float x, float y, float w, float h, const RefCntAutoPtr<ITexture> &texture, float radius, const ImageDrawMode& imageDrawMode, const SpriteBorder& spriteBorder)
	{
		switch (imageDrawMode) {
			case ImageDrawMode::Simple: {
				DrawArgs drawArgs;
				drawArgs.Texture = texture;
				drawArgs.Radius = radius;
				drawArgs.Fill = true;
				Draw(x, y, w, h, drawArgs);
				break;
			}
			case ImageDrawMode::Sliced: {
				DrawArgs drawArgs;
				drawArgs.Texture = texture;
				drawArgs.Radius = radius;
				drawArgs.Fill = true;
				drawArgs.EnabledCustomUVs = true;

				auto realW = static_cast<float>(texture->GetDesc().Width);
				auto realH = static_cast<float>(texture->GetDesc().Height);

				float borderLeftPercent = spriteBorder.Left / realW;
				float borderRightPercent = spriteBorder.Right / realW;
				float borderTopPercent = spriteBorder.Top / realH;
				float borderBottomPercent = spriteBorder.Bottom / realH;

				float borderLeft = spriteBorder.Left;
				float borderRight = spriteBorder.Right;
				float borderTop = spriteBorder.Top;
				float borderBottom = spriteBorder.Bottom;

				/*
				 *  drawArgs.CustomUVs[0] = Vector2(1, 0);
					drawArgs.CustomUVs[1] = Vector2(0, 0);
					drawArgs.CustomUVs[2] = Vector2(1, 1);
					drawArgs.CustomUVs[3] = Vector2(0, 1);
				 */

				{ // Corners
					{ // Left top corner
						drawArgs.CustomUVs[0] = Vector2(borderLeftPercent, 0);
						drawArgs.CustomUVs[1] = Vector2(0, 0);
						drawArgs.CustomUVs[2] = Vector2(borderLeftPercent, borderTopPercent);
						drawArgs.CustomUVs[3] = Vector2(0, borderTopPercent);
						Draw(x, y, borderLeft, borderTop, drawArgs);
					}

					{ // Right top corner
						drawArgs.CustomUVs[0] = Vector2(1, 0);
						drawArgs.CustomUVs[1] = Vector2(1.0 - borderRightPercent, 0);
						drawArgs.CustomUVs[2] = Vector2(1, borderTopPercent);
						drawArgs.CustomUVs[3] = Vector2(1.0 - borderRightPercent, borderTopPercent);
						Draw(x + w - borderRight, y, borderLeft, borderTop, drawArgs);
					}

					{ // left bottom corner
						drawArgs.CustomUVs[0] = Vector2(borderLeftPercent, 1.0 - borderBottomPercent);
						drawArgs.CustomUVs[1] = Vector2(0, 1.0 - borderBottomPercent);
						drawArgs.CustomUVs[2] = Vector2(borderLeftPercent, 1);
						drawArgs.CustomUVs[3] = Vector2(0, 1);
						Draw(x, y + h - borderBottom, borderRight, borderBottom, drawArgs);
					}

					{ // right bottom corner
						drawArgs.CustomUVs[0] = Vector2(1, 1.0 - borderBottomPercent);
						drawArgs.CustomUVs[1] = Vector2(1.0 - borderRightPercent, 1.0 - borderBottomPercent);
						drawArgs.CustomUVs[2] = Vector2(1, 1);
						drawArgs.CustomUVs[3] = Vector2(1.0 - borderRightPercent, 1);
						Draw(x + w - borderRight, y + h - borderBottom, borderRight, borderBottom, drawArgs);
					}
				}

				{ // Middle
					drawArgs.CustomUVs[0] = Vector2(1 - borderRightPercent, borderTopPercent);
					drawArgs.CustomUVs[1] = Vector2(borderLeftPercent, borderTopPercent);
					drawArgs.CustomUVs[2] = Vector2(1 - borderRightPercent, 1 - borderBottomPercent);
					drawArgs.CustomUVs[3] = Vector2(borderLeftPercent, 1 - borderBottomPercent);
					Draw(x + borderLeft, y + borderTop, w - borderLeft - borderRight, h - borderTop - borderBottom, drawArgs);
				}

				{ // Sides
					{ // Left side
						drawArgs.CustomUVs[0] = Vector2(borderLeftPercent, borderTopPercent);
						drawArgs.CustomUVs[1] = Vector2(0, borderTopPercent);
						drawArgs.CustomUVs[2] = Vector2(borderLeftPercent, 1 - borderBottomPercent);
						drawArgs.CustomUVs[3] = Vector2(0, 1 - borderBottomPercent);
						Draw(x, y + borderTop, borderLeft, h - borderTop - borderBottom, drawArgs);
					}

					{ // Right side
						drawArgs.CustomUVs[0] = Vector2(1, borderTopPercent);
						drawArgs.CustomUVs[1] = Vector2(1.0 - borderRightPercent, borderTopPercent);
						drawArgs.CustomUVs[2] = Vector2(1, 1 - borderBottomPercent);
						drawArgs.CustomUVs[3] = Vector2(1.0 - borderRightPercent, 1 - borderBottomPercent);
						Draw(x + w - borderRight, y + borderTop, borderLeft, h - borderTop - borderBottom, drawArgs);
					}

					{ // Top side
						drawArgs.CustomUVs[0] = Vector2(1 - borderRightPercent, 0);
						drawArgs.CustomUVs[1] = Vector2(borderLeftPercent, 0);
						drawArgs.CustomUVs[2] = Vector2(1 - borderRightPercent, borderTopPercent);
						drawArgs.CustomUVs[3] = Vector2(borderLeftPercent, borderTopPercent);
						Draw(x + borderLeft, y, w - borderLeft - borderRight, borderTop, drawArgs);
					}

					{ // Bottom side
						drawArgs.CustomUVs[0] = Vector2(1 - borderRightPercent, 1 - borderBottomPercent);
						drawArgs.CustomUVs[1] = Vector2(borderLeftPercent, 1.0 - borderBottomPercent);
						drawArgs.CustomUVs[2] = Vector2(1 - borderRightPercent, 1);
						drawArgs.CustomUVs[3] = Vector2(borderLeftPercent, 1);
						Draw(x + borderLeft, y + h - borderBottom, w - borderLeft - borderRight, borderBottom, drawArgs);
					}
				}

				break;
			}
			case ImageDrawMode::Tiled: {
				//TODO: Finish tiled image drawing
				break;
			}
		}
	}

	void UIRenderer::SetImageEdgeDetection(bool enabled, int thickness, const Vector4& edgeColor)
	{
		m_Material->SetVariable<int>("ImageEdgeDetectionEnabled", enabled);
		if(enabled) {
			m_Material->SetVariable<int>("EdgeThickness", thickness);
			m_Material->SetVariable<Vector4>("EdgeColor", edgeColor);
		}
	}
}