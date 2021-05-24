#include "UIRenderer.hpp"
#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{
	UIRenderer::UIRenderer() : m_Material(nullptr), m_ProjectionMatrix(), m_LastMaterial(nullptr), m_Fonts(), m_CurrentFont("Default")
	{
		/*{
			m_Material = std::make_shared<Material>("UI", "Assets/Shaders/UI");
			m_Material->SetCullMode(CULL_MODE_NONE);
			m_Material->SetDepthEnable(false);
			m_Material->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
			RenderTargetBlendDesc blendDesc;
			blendDesc.BlendEnable = true;
			blendDesc.SrcBlend = BLEND_FACTOR_SRC_ALPHA;
			blendDesc.DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
			m_Material->SetBlendState(blendDesc);
			//m_Material->SetFillMode(FILL_MODE_WIREFRAME);
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
		}*/

		LoadFont("Default", "Assets/Fonts/GROBOLD.ttf");
	}

	void UIRenderer::Begin(const Vector2i& size)
	{
		if(size.x > 0 && size.y > 0) {
			m_ProjectionMatrix = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -1.0f, 1.0f);
		}

		/*{
			GraphicsPipelineDesc& graphicsPipelineDesc = m_Material->GetPipelineDesc();
			graphicsPipelineDesc.NumRenderTargets = 1;
			graphicsPipelineDesc.RTVFormats[0] = textureFormat;
			graphicsPipelineDesc.DSVFormat = depthFormat;
			m_Material->ValidateGraphicsPipelineState();
			//m_Material->ApplyPipeline();
		}

		{
			GraphicsPipelineDesc& graphicsPipelineDesc = m_FontMaterial->GetPipelineDesc();
			graphicsPipelineDesc.NumRenderTargets = 1;
			graphicsPipelineDesc.RTVFormats[0] = textureFormat;
			graphicsPipelineDesc.DSVFormat = depthFormat;
			m_FontMaterial->ValidateGraphicsPipelineState();
		}*/

		SetImageEdgeDetection(false, 3);
	}

	void UIRenderer::End()
	{

	}

	void UIRenderer::FillRect(float x, float y, float w, float h, const Vector4 &color, float radius, float rotation)
	{
		DrawArgs drawArgs;
		drawArgs.Color = color;
		drawArgs.Radius = radius;
		drawArgs.Rotation = rotation;
		Draw(x, y, w, h, drawArgs);
	}

	void UIRenderer::FillRect(const Vector2 &position, const Vector2 &size, const Vector4 &color, float radius, float rotation)
	{
		FillRect(position.x, position.y, size.x, size.y, color, radius, rotation);
	}

	void UIRenderer::DrawRect(float x, float y, float w, float h, const Vector4& color, float strokeSize, float radius, float rotation)
	{
		DrawArgs drawArgs;
		drawArgs.Color = color;
		drawArgs.Fill = false;
		drawArgs.StrokeSize = strokeSize + 1.0f;
		drawArgs.Radius = radius;
		drawArgs.Rotation = rotation;
		Draw(x, y, w, h, drawArgs);
	}

	void UIRenderer::DrawRect(const Vector2 &position, const Vector2 &size, const Vector4 &color, float strokeSize, float radius, float rotation)
	{
		DrawRect(position.x, position.y, size.x, size.y, color, strokeSize, radius, rotation);
	}

	void UIRenderer::Draw(float x, float y, float w, float h, const DrawArgs& drawArgs)
	{
		Material_ptr material = drawArgs.OverrideMaterial == nullptr ? m_Material : drawArgs.OverrideMaterial;

		if(m_LastMaterial != material) {
			m_LastMaterial = material;
			//material->ValidateGraphicsPipelineState();

		}

		material->ApplyPipeline();

		if(!drawArgs.Fill) {
			float strokeHalf = drawArgs.StrokeSize / 2.0f;
			if(strokeHalf < 1.0) strokeHalf = 1.0f;
			x -= strokeHalf;
			y -= strokeHalf;
			w += strokeHalf * 2.0f;
			h += strokeHalf * 2.0f;
		}

		material->SetVariable<Matrix4>("Projection", m_ProjectionMatrix);
		material->SetVariable<Matrix4>("ModelMat", glm::translate(Vector3(x + w / 2.0f, y + h / 2.0f, 0)) * glm::rotate(glm::radians(drawArgs.Rotation), Vector3(0, 0, 1)) * glm::scale(Vector3(w, h, 1)));

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
		material->SetVariable<float>("GrayScale", 0.0f); // TODO: Add this as setting

		material->CommitShaderResources();

		/*DrawAttribs drawAttrs;
		drawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
		drawAttrs.NumVertices = 4;
		AuroraEngine::ImmediateContext->Draw(drawAttrs);*/
	}

	void UIRenderer::DrawImage(float x, float y, float w, float h, Texture_ptr texture, float radius, const ImageDrawMode& imageDrawMode, const SpriteBorder& spriteBorder, const Color& tint)
	{
		switch (imageDrawMode) {
			case ImageDrawMode::Simple: {
				DrawArgs drawArgs;
				drawArgs.Texture = texture;
				drawArgs.Radius = radius;
				drawArgs.Fill = true;
				drawArgs.Tint = tint;
				Draw(x, y, w, h, drawArgs);
				break;
			}
			case ImageDrawMode::Sliced: {
				DrawArgs drawArgs;
				drawArgs.Texture = texture;
				drawArgs.Radius = radius;
				drawArgs.Fill = true;
				drawArgs.EnabledCustomUVs = true;
				drawArgs.Tint = tint;

				TextureDesc textureDesc = AuroraEngine::RenderDevice->describeTexture(texture);

				auto realW = static_cast<float>(textureDesc.Width);
				auto realH = static_cast<float>(textureDesc.Height);

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

	void UIRenderer::DrawImage(const Vector2 &position, const Vector2 &size, Texture_ptr texture, float radius, const ImageDrawMode &imageDrawMode, const SpriteBorder &spriteBorder, const Color& tint)
	{
		DrawImage(position.x, position.y, size.x, size.y, texture, radius, imageDrawMode, spriteBorder, tint);
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
		if(fontData.empty()) {
			return false;
		}

		m_Fonts[name] = std::make_shared<Font>(name, fontData);
		return true;
	}

	void UIRenderer::Text(const String& text, float x, float y, float fontSize, const Vector4& color, const String& fontName)
	{
		Font_ptr font = FindFont(fontName);

		if(font == nullptr) {
			AU_LOG_ERROR("Cannot find font ", fontName);
			return;
		}

		SetFont(fontName);

		FontSize_t optimalFontSize = font->FindSuitableSize(fontSize);

		FontBitmapPageList_ptr fontBitmapPageList = font->FindOrCreatePageList(optimalFontSize);

		DrawArgs drawArgs;
		drawArgs.Radius = 0;
		drawArgs.Fill = true;
		drawArgs.EnabledCustomUVs = true;
		drawArgs.OverrideMaterial = m_FontMaterial;

		float fontScale = fontSize / static_cast<float>(optimalFontSize);

		float baseLine = 0;

		/*{ // Sets the baseline
			FontGlyph glyph = {};
			for(char c : text) {
				if(!fontBitmapPageList->FindGlyph(c, glyph)) {
					continue;
				}

				baseLine = glm::min(baseLine, glyph.yOff);
			}

			y += glm::abs(baseLine) * fontScale;
		}*/

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
				Draw(bakedRect.x, bakedRect.y + bakedRect.Baseline, bakedRect.width, bakedRect.height, drawArgs);

				// Move to next char position
				x += xAdvance;
			}
		}
	}

	void UIRenderer::Text(const String &text, const Vector2 &position, float fontSize, const Vector4 &color, const String &fontName)
	{
		Text(text, position.x, position.y, fontSize, color, fontName);
	}

	Vector2 UIRenderer::GetTextSize(const String& text, float fontSize, const String& fontName)
	{
		if(text.empty()) {
			return Vector2(0, 0);
		}

		Font_ptr font = FindFont(fontName);

		if(font == nullptr) {
			AU_LOG_ERROR("Cannot find font ", fontName);
			return {0, 0};
		}

		FontSize_t optimalFontSize = font->FindSuitableSize(fontSize);
		FontBitmapPageList_ptr fontBitmapPageList = font->FindOrCreatePageList(optimalFontSize);

		float fontScale = fontSize / static_cast<float>(optimalFontSize);

		Vector2 min = std::numeric_limits<Vector2>::infinity();
		Vector2 max = -min;

		FontGlyph glyph = {};
		float x = 0;
		float y = 0;
		float maxX = 0;

		for(char c : text) {
			if(!fontBitmapPageList->FindGlyph(c, glyph)) {
				continue;
			}

			min.x = std::min<float>(min.x, x);
			min.y = std::min<float>(min.y, y);

			//x = x + glyph.xOff * fontScale;
			y = (glyph.yOff * fontScale) + glyph.Baseline;
			//y = glyph.Baseline;

			float w = static_cast<float>(glyph.Width) * fontScale;
			float h = static_cast<float>(glyph.Height) * fontScale;

			min.x = std::min<float>(min.x, x);
			min.y = std::min<float>(min.y, y);

			max.x = std::max<float>(max.x, x + w);
			max.y = std::max<float>(max.y, y + h);

			maxX = x + w;

			x += (glyph.xAdvance * fontScale);
		}

		max.x = maxX;

		max.x -= 1;
		max.y -= 1;

		return glm::abs(max - min);
	}
}