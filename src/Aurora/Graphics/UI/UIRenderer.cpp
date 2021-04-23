#include "UIRenderer.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace Aurora
{
	unsigned char temp_bitmap[512*512];
	stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs

	RefCntAutoPtr<ITexture> FontTexture;

	RefCntAutoPtr<ITexture> CreateFontTexture(int w, int h, const unsigned char* data)
	{
		TextureDesc TexDesc;
		TexDesc.Name      = "RandomNoise";
		TexDesc.Type      = RESOURCE_DIM_TEX_2D;
		TexDesc.Width     = w;
		TexDesc.Height    = h;
		TexDesc.MipLevels = 1;
		TexDesc.Format    = TEX_FORMAT_R8_UNORM;
		// The render target can be bound as a shader resource and as a render target
		TexDesc.BindFlags = BIND_RENDER_TARGET;

		TexDesc.BindFlags |= BIND_SHADER_RESOURCE;

		std::vector<TextureSubResData>  pSubResources(TexDesc.MipLevels);
		pSubResources[0].pData  = data;
		pSubResources[0].Stride = sizeof(unsigned char) * w;

		TextureData TexData;
		TexData.pSubResources   = pSubResources.data();
		TexData.NumSubresources = TexDesc.MipLevels;

		RefCntAutoPtr<ITexture> pRTColor;
		AuroraEngine::RenderDevice->CreateTexture(TexDesc, &TexData, &pRTColor);

		return pRTColor;
	}

	UIRenderer::UIRenderer() : m_Material(nullptr), m_ProjectionMatrix(), m_LastMaterial(nullptr)
	{
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

		{
			m_FontMaterial = std::make_shared<Material>("UI", "Assets/Shaders/UI_Font");
			m_FontMaterial->SetCullMode(CULL_MODE_NONE);
			m_FontMaterial->SetDepthEnable(false);
			m_FontMaterial->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
			RenderTargetBlendDesc blendDesc;
			blendDesc.BlendEnable = true;
			blendDesc.SrcBlend = BLEND_FACTOR_SRC_ALPHA;
			blendDesc.DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
			m_FontMaterial->SetBlendState(blendDesc);
		}

		auto fontData = AuroraEngine::AssetManager->LoadFile("Assets/Fonts/GROBOLD.ttf");
		if(fontData != nullptr) {
			stbtt_BakeFontBitmap(reinterpret_cast<unsigned char *>(fontData->GetDataPtr()),0, 32.0, temp_bitmap,512,512, 32,96, cdata);

			FontTexture = CreateFontTexture(512, 512, temp_bitmap);
		}
	}

	void UIRenderer::Begin(const Vector2i& size, const TEXTURE_FORMAT& textureFormat, const TEXTURE_FORMAT& depthFormat)
	{
		m_ProjectionMatrix = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -1.0f, 1.0f);

		{
			GraphicsPipelineDesc& graphicsPipelineDesc = m_Material->GetPipelineDesc();
			graphicsPipelineDesc.NumRenderTargets = 1;
			graphicsPipelineDesc.RTVFormats[0] = textureFormat;
			graphicsPipelineDesc.DSVFormat = depthFormat;
			m_Material->ValidateGraphicsPipelineState();
			m_Material->ApplyPipeline();
		}

		{
			GraphicsPipelineDesc& graphicsPipelineDesc = m_FontMaterial->GetPipelineDesc();
			graphicsPipelineDesc.NumRenderTargets = 1;
			graphicsPipelineDesc.RTVFormats[0] = textureFormat;
			graphicsPipelineDesc.DSVFormat = depthFormat;
			m_FontMaterial->ValidateGraphicsPipelineState();
		}

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
		Material_ptr material = drawArgs.OverrideMaterial == nullptr ? m_Material : drawArgs.OverrideMaterial;

		if(m_LastMaterial != material) {
			m_LastMaterial = material;
			material->ValidateGraphicsPipelineState();
			material->ApplyPipeline();
		}

		if(!drawArgs.Fill) {
			float strokeHalf = drawArgs.StrokeSize / 2.0f;
			x -= strokeHalf;
			y -= strokeHalf;
			w += strokeHalf * 2.0f;
			h += strokeHalf * 2.0f;
		}

		material->SetVariable<Matrix4>("Projection", m_ProjectionMatrix);
		material->SetVariable<Matrix4>("ModelMat", glm::translate(Vector3(x, y, 0)) * glm::scale(Vector3(w, h, 1)));

		if(drawArgs.EnabledCustomUVs) {
			static Vector4 cacheUvVal[4];

			for (int i = 0; i < 4; ++i) {
				cacheUvVal[i].x = drawArgs.CustomUVs[i].x;
				cacheUvVal[i].y = drawArgs.CustomUVs[i].y;
			}

			material->SetArray("UVs", cacheUvVal, sizeof(Vector4) * 4);
			material->SetVariable<int>("CustomUVsEnabled", true);
		} else {
			material->SetVariable<int>("CustomUVsEnabled", false);
		}

		if(drawArgs.Texture != nullptr) {
			material->SetVariable<uint32_t>("Type", 0);
			material->SetTexture("Texture", drawArgs.Texture);
			material->SetVariable<Vector4>("Color", drawArgs.Tint);
		} else {
			if(drawArgs.Fill) {
				material->SetVariable<uint32_t>("Type", 1);
			} else {
				material->SetVariable<uint32_t>("Type", 2);
				material->SetVariable<float>("StrokeSize", drawArgs.StrokeSize / 2.0f);
			}

			material->SetVariable<Vector4>("Color", drawArgs.Color);
		}

		material->SetVariable<Vector2>("Size", Vector2(w, h));
		material->SetVariable<float>("Radius", drawArgs.Radius);

		material->CommitShaderResources();

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

	void UIRenderer::Text(const String &text, float x, float y, const Vector4 &color)
	{
		DrawArgs drawArgs;
		drawArgs.Texture = FontTexture;
		drawArgs.Radius = 0;
		drawArgs.Fill = true;
		drawArgs.EnabledCustomUVs = true;
		drawArgs.OverrideMaterial = m_FontMaterial;

		for(char c : text) {
			if (c >= 32 && c < 128) {
				stbtt_aligned_quad q;
				stbtt_GetBakedQuad(cdata, 512,512, c-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9

				drawArgs.CustomUVs[0] = Vector2(q.s1, q.t0);
				drawArgs.CustomUVs[1] = Vector2(q.s0, q.t0);
				drawArgs.CustomUVs[2] = Vector2(q.s1, q.t1);
				drawArgs.CustomUVs[3] = Vector2(q.s0, q.t1);

				float gx = q.x0;
				float gy = q.y0;

				float gw = q.x1 - q.x0;
				float gh = q.y1 - q.y0;

				// We can set custom color to every character,
				// for now we just set the color same for every character
				drawArgs.Tint = color;

				Draw(gx, gy, gw, gh, drawArgs);
			}
		}
	}
}