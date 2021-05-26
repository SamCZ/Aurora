#include "UIRenderer.hpp"
#include "Aurora/AuroraEngine.hpp"

#include <glad/glad.h>

namespace Aurora
{
	using namespace glm;

	// Vertex uniform blocks

	struct VertexUniform
	{
		mat4 Projection;
		mat4  ModelMat;
	};

	struct UVData
	{
		vec4 FirstData;
		vec2 UV0;
		vec2 UV1;
		vec2 UV2;
		vec2 UV3;
	};

	// Fragment uniform blocks

	struct FragmentUniform
	{
		uint Type;
		float StrokeSize;
		float Radius;
		int EdgeThickness;
		vec4 Color;
		vec2 Size;
		float GrayScale;
		int ImageEdgeDetectionEnabled = 0;
		vec4 EdgeColor;
	};

	struct FontData
	{
		vec4 Color;
	};

	Shader_ptr ui_vertex;
	Shader_ptr ui_pixel;

	Shader_ptr ui_font_vertex;
	Shader_ptr ui_font_pixel;

	SamplerHandle sampler;

	UniformBufferHandle vertexUniformBuffer;
	UniformBufferHandle uvDataUniformBuffer;
	UniformBufferHandle fragmentUniformUniformBuffer;
	UniformBufferHandle fontDataUniformBuffer;

	UIRenderer::UIRenderer() : m_Material(nullptr), m_ProjectionMatrix(), m_LastMaterial(nullptr), m_Fonts(), m_CurrentFont("Default")
	{
		LoadFont("Default", "Assets/Fonts/GROBOLD.ttf");

		/*ui_vertex = ASM->LoadShaderResource("Assets/Shaders/UI/vertex.glsl", ShaderType::Vertex)->GetOrCompile().Shader;
		ui_pixel = ASM->LoadShaderResource("Assets/Shaders/UI/fragment.glsl", ShaderType::Pixel)->GetOrCompile().Shader;

		ui_font_vertex = ASM->LoadShaderResource("Assets/Shaders/UI_Font/vertex.glsl", ShaderType::Vertex)->GetOrCompile().Shader;
		ui_font_pixel = ASM->LoadShaderResource("Assets/Shaders/UI_Font/fragment.glsl", ShaderType::Pixel)->GetOrCompile().Shader;

		SamplerDesc samplerDesc;
		sampler = AuroraEngine::RenderDevice->CreateSampler(samplerDesc);

		vertexUniformBuffer = AuroraEngine::RenderDevice->CreateUniformBuffer(UniformBufferDesc(sizeof(VertexUniform), "VertexUniform"), nullptr);
		uvDataUniformBuffer = AuroraEngine::RenderDevice->CreateUniformBuffer(UniformBufferDesc(sizeof(UVData), "UVData"), nullptr);
		fragmentUniformUniformBuffer = AuroraEngine::RenderDevice->CreateUniformBuffer(UniformBufferDesc(sizeof(FragmentUniform), "FragmentUniform"), nullptr);
		fontDataUniformBuffer = AuroraEngine::RenderDevice->CreateUniformBuffer(UniformBufferDesc(sizeof(FontData), "FontData"), nullptr);*/
	}

	void UIRenderer::Begin(const Vector2i& size)
	{
		if(size.x > 0 && size.y > 0) {
			m_ProjectionMatrix = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, -1.0f, 1.0f);
		}
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
		/*if(!drawArgs.Fill) {
			float strokeHalf = drawArgs.StrokeSize / 2.0f;
			if(strokeHalf < 1.0) strokeHalf = 1.0f;
			x -= strokeHalf;
			y -= strokeHalf;
			w += strokeHalf * 2.0f;
			h += strokeHalf * 2.0f;
		}

		{ // Set matrices
			VertexUniform vertexUniform = {};
			vertexUniform.Projection = m_ProjectionMatrix;
			vertexUniform.ModelMat = glm::translate(Vector3(x + w / 2.0f, y + h / 2.0f, 0)) * glm::rotate(glm::radians(drawArgs.Rotation), Vector3(0, 0, 1)) * glm::scale(Vector3(w, h, 1));

			AuroraEngine::RenderDevice->WriteUniformBuffer(vertexUniformBuffer, &vertexUniform, sizeof(VertexUniform), 0);
		}

		{ // UV data
			UVData uvData = {};
			if(drawArgs.EnabledCustomUVs) {
				uvData.FirstData.x = 1;

				uvData.UV0 = drawArgs.CustomUVs[0];
				uvData.UV1 = drawArgs.CustomUVs[1];
				uvData.UV2 = drawArgs.CustomUVs[2];
				uvData.UV3 = drawArgs.CustomUVs[3];

			} else {
				uvData.FirstData.x = 0;
			}

			uvData.FirstData.z = 0;

			AuroraEngine::RenderDevice->WriteUniformBuffer(uvDataUniformBuffer, &uvData, sizeof(UVData), 0);
		}

		if (!drawArgs.IsFont) { // Fragment data
			FragmentUniform fragmentUniform = {};

			if(drawArgs.Texture != nullptr) {
				fragmentUniform.Type = 0;
				fragmentUniform.Color = drawArgs.Tint;
			} else {
				if(drawArgs.Fill) {
					fragmentUniform.Type = 1;
				} else {
					fragmentUniform.Type = 2;
					fragmentUniform.StrokeSize = drawArgs.StrokeSize / 2.0f;
				}

				fragmentUniform.Color = drawArgs.Color;
			}

			fragmentUniform.Size = Vector2(w, h);
			fragmentUniform.Radius = drawArgs.Radius;
			fragmentUniform.GrayScale = 0.0f;

			AuroraEngine::RenderDevice->WriteUniformBuffer(fragmentUniformUniformBuffer, &fragmentUniform, sizeof(FragmentUniform), 0);
		} else {
			FontData fontData = {};
			fontData.Color = drawArgs.Color;

			AuroraEngine::RenderDevice->WriteUniformBuffer(fontDataUniformBuffer, &fontData, sizeof(FontData), 0);
		}

		DrawCallState state;
		state.primType = PrimitiveType::TriangleStrip;

		if(!drawArgs.IsFont) {
			state.VS.shader = ui_vertex;
			state.PS.shader = ui_pixel;
		} else {
			state.VS.shader = ui_font_vertex;
			state.PS.shader = ui_font_pixel;
		}

		state.PS.BoundTextures["Texture"] = drawArgs.Texture;
		state.PS.BoundSamplers["Texture"] = sampler;
		if(!drawArgs.IsFont) {
			state.PS.BoundUniformBuffers["FragmentUniform"] = fragmentUniformUniformBuffer;
		} else {
			state.PS.BoundUniformBuffers["FontData"] = fontDataUniformBuffer;
		}

		state.VS.BoundUniformBuffers["VertexUniform"] = vertexUniformBuffer;
		state.VS.BoundUniformBuffers["UVData"] = uvDataUniformBuffer;

		state.renderState.targetCount = 0;
		state.renderState.targets[0] = nullptr;

		state.renderState.depthStencilState.depthEnable = false;
		state.renderState.rasterState.cullMode = RasterState::CullMode::None;

		state.renderState.clearColorTarget = false;
		state.renderState.clearDepthTarget = false;
		state.renderState.clearStencilTarget = false;

		if(state.renderState.targetCount > 0) {
			BlendState blendState;

			for (uint32_t i = 0; i < BlendState::MAX_MRT_BLEND_COUNT; i++)
			{
				blendState.blendEnable[i] = true;
				blendState.colorWriteEnable[i] = BlendState::COLOR_MASK_ALL;

				// TODO: Check if this right settings for alpha
				blendState.blendOpAlpha[i] = BlendState::BlendOp::BLEND_OP_ADD;
				blendState.srcBlendAlpha[i] = BlendState::BlendValue::BLEND_SRC_ALPHA;
				blendState.destBlendAlpha[i] = BlendState::BlendValue::BLEND_INV_SRC_ALPHA;

				blendState.blendOp[i] = BlendState::BlendOp::BLEND_OP_ADD;
				blendState.srcBlend[i] = BlendState::BlendValue::BLEND_SRC_ALPHA;
				blendState.destBlend[i] = BlendState::BlendValue::BLEND_INV_SRC_ALPHA;
			}

			state.renderState.blendState = blendState;
		} else {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		DrawArguments args;
		args.instanceCount = 1;
		args.vertexCount = 4;
		AuroraEngine::RenderDevice->Draw(state, &args, 1);*/

		/*Material_ptr material = drawArgs.OverrideMaterial == nullptr ? m_Material : drawArgs.OverrideMaterial;

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

		material->CommitShaderResources();*/

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

				TextureDesc textureDesc;
				//TextureDesc textureDesc = AuroraEngine::RenderDevice->DescribeTexture(texture);

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
		/*m_Material->SetVariable<int>("ImageEdgeDetectionEnabled", enabled);
		if(enabled) {
			m_Material->SetVariable<int>("EdgeThickness", thickness);
			m_Material->SetVariable<Vector4>("EdgeColor", edgeColor);
		}*/
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
		drawArgs.IsFont = true;

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