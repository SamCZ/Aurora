#include "UIRenderer.hpp"

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

	UIRenderer::UIRenderer() : m_Material(nullptr), m_ProjectionMatrix(), m_LastMaterial(nullptr), m_Fonts()
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
			//m_FontMaterial->SetFillMode(FILL_MODE_WIREFRAME);
		}

		LoadFont("Default", "Assets/Fonts/GROBOLD.ttf");

		auto fontData = AuroraEngine::AssetManager->LoadFile("Assets/Fonts/GROBOLD.ttf");
		if(fontData != nullptr) {
			// 0x0020 - 0x00FF

			stbtt_BakeFontBitmap(reinterpret_cast<unsigned char *>(fontData->GetDataPtr()),0, 16.0, temp_bitmap,512,512, 32,96, cdata);
			FontTexture = CreateFontTexture(512, 512, temp_bitmap);




			//CalcTextSize
		}
	}

	void UIRenderer::Begin(const Vector2i& size, const TEXTURE_FORMAT& textureFormat, const TEXTURE_FORMAT& depthFormat)
	{
		if(size.x > 0 && size.y > 0) {
			m_ProjectionMatrix = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -1.0f, 1.0f);
		}

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

	Font_ptr UIRenderer::FindFont(const String &name)
	{
		auto iter = m_Fonts.find(name);

		if(iter != m_Fonts.end()) {
			return iter->second;
		}

		return nullptr;
	}

	bool UIRenderer::LoadFont(const String &name, const Path &path)
	{
		auto fontData = AuroraEngine::AssetManager->LoadFile(path);
		if(fontData == nullptr) {
			return false;
		}

		m_Fonts[name] = std::make_shared<Font>(name, fontData);
		return true;
	}

	void UIRenderer::Text(const String& text, float x, float y, float fontSize, const Vector4& color, const String& fontName)
	{
		Font_ptr font = FindFont(fontName);

		if(font == nullptr) {
			AU_DEBUG_CERR("Cannot find font " << fontName);
			return;
		}

		FontSize_t optimalFontSize = font->FindSuitableSize(fontSize);

		FontBitmapPageList_ptr fontBitmapPageList = font->FindOrCreatePageList(optimalFontSize);

		DrawArgs drawArgs;
		drawArgs.Radius = 0;
		drawArgs.Fill = true;
		drawArgs.EnabledCustomUVs = true;
		drawArgs.OverrideMaterial = m_FontMaterial;

		//auto textSize = GetTextSize(text, fontSize, fontName);

		float fontScale = fontSize / static_cast<float>(optimalFontSize);

		float baseLine = 0;


		{ // Sets the baseline
			for(char c : text) {
				FontGlyph firstGlyph = {};
				if(!fontBitmapPageList->FindGlyph(c, firstGlyph)) {
					continue;
				}

				baseLine = glm::min(baseLine, firstGlyph.yOff);
			}

			y += glm::abs(baseLine) * fontScale;
		}

		BakedRect bakedRect = {};
		float xAdvance = 0;

		// TODO: convert to UTF32
		for(char c : text) {
			if(fontBitmapPageList->GetBakedRectForCodepoint(c, x, y, bakedRect, xAdvance, fontScale)) {
				// Set UVs
				drawArgs.CustomUVs[0] = Vector2(bakedRect.RightBottomUV.x, bakedRect.LeftTopUV.y);
				drawArgs.CustomUVs[1] = Vector2(bakedRect.LeftTopUV.x, bakedRect.LeftTopUV.y);
				drawArgs.CustomUVs[2] = Vector2(bakedRect.RightBottomUV.x, bakedRect.RightBottomUV.y);
				drawArgs.CustomUVs[3] = Vector2(bakedRect.LeftTopUV.x, bakedRect.RightBottomUV.y);

				// We can set custom color to every character,
				// for now we just set the color same for every character
				drawArgs.Tint = color;

				// Set the page texture
				drawArgs.Texture = bakedRect.Texture;

				// Draw rect
				Draw(bakedRect.x, bakedRect.y, bakedRect.width, bakedRect.height, drawArgs);

				// Move to next char position
				x += xAdvance;
			}
		}
	}

	Vector2 UIRenderer::GetTextSize(const String& text, float fontSize, const String& font)
	{
		// TODO: Fix to new font system

		Vector2 min = {999999, 999999};
		Vector2 max = {-999999, -999999};

		float x = 0, y = 0;

		for(char c : text) {
			if (c >= 32 && c < 128) {
				stbtt_aligned_quad q;
				stbtt_GetBakedQuad(cdata, 512,512, c-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9

				min.x = std::min<float>(min.x, q.x0);
				min.y = std::min<float>(min.y, q.y0);

				max.x = std::max<float>(max.x, q.x1);
				max.y = std::max<float>(max.y, q.y1);
			}
		}

		//std::cout << glm::to_string(min) << " - " << glm::to_string(max) << std::endl;

		return max - min;
	}
}