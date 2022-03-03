#include "VgRender.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

#include "Aurora/Graphics/OpenGL/GL.hpp"
#include "Aurora/Graphics/OpenGL/GLRenderDevice.hpp"
#include "Aurora/Graphics/NanoVG/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "Aurora/Graphics/NanoVG/nanovg_gl.h"

namespace Aurora
{
	inline NVGcolor ColorToVg(Color color)
	{
		return nvgRGBA(color.r, color.g, color.b, color.a);
	}

	VgRender::VgRender()
	{
		m_VgContext = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES); //NVG_DEBUG
	}

	VgRender::~VgRender()
	{
		nvgDeleteGL3(m_VgContext);
	}

	void VgRender::Begin(const Vector2ui &screenSize, float devicePixelRatio)
	{
		nvgBeginFrame(m_VgContext, static_cast<float>(screenSize.x), static_cast<float>(screenSize.y), devicePixelRatio);
	}

	void VgRender::End()
	{
		nvgEndFrame(m_VgContext);
	}

	bool VgRender::LoadFont(const String &name, const Path &path)
	{
		auto fontData = GEngine->GetResourceManager()->LoadFile(path);

		if(fontData.empty())
		{
			AU_LOG_WARNING("Could not load font ", path.string());
			return false;
		}

		// calloc needs to be called, because NanoVg uses c-style free() fn
		uint8_t* fontMemData = (uint8_t*)calloc(fontData.size(), sizeof(uint8_t));
		memcpy(fontMemData, fontData.data(), fontData.size());

		int fontIdx = nvgCreateFontMem(m_VgContext, name.c_str(), fontMemData, static_cast<int>(fontData.size()), 1);

		if(fontIdx >= 0)
		{
			AU_LOG_INFO("Font loaded ", name, path.string());
			return true;
		}

		AU_LOG_WARNING("Could not parse font ", path.string());

		return false;
	}

	void VgRender::DrawString(const String &text, const Vector2 &pos, Color color, float fontSize, VgAlign xAlign, VgAlign yAlign)
	{
		nvgFontSize(m_VgContext, static_cast<float>(fontSize));
		nvgFontFace(m_VgContext, "default");

		uint8_t flags = 0;
		switch (xAlign)
		{
			case VgAlign::Left:
				flags |= NVG_ALIGN_LEFT;
				break;
			case VgAlign::Right:
				flags |= NVG_ALIGN_RIGHT;
				break;
			case VgAlign::Center:
				flags |= NVG_ALIGN_CENTER;
				break;
			default:
				break;
		}

		switch (yAlign)
		{
			case VgAlign::Top:
				flags |= NVG_ALIGN_TOP;
				break;
			case VgAlign::Bottom:
				flags |= NVG_ALIGN_BOTTOM;
				break;
			case VgAlign::Center:
				flags |= NVG_ALIGN_MIDDLE;
				break;
			default:
				break;
		}

		nvgFontBlur(m_VgContext, 0);
		nvgGlobalAlpha(m_VgContext, 1.0f);
		nvgTextAlign(m_VgContext, flags);
		nvgFillColor(m_VgContext, ColorToVg(color));
		nvgText(m_VgContext, pos.x, pos.y, text.c_str(), nullptr);
	}

	float VgRender::GetTextSize(const String &text, float fontSize, const String &fontName, float *textHeight)
	{
		nvgFontSize(m_VgContext, fontSize);
		nvgFontFace(m_VgContext, fontName.c_str());
		nvgTextAlign(m_VgContext, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

		float w = nvgTextBounds(m_VgContext, 0.0f, 0.0f, text.c_str(), NULL, m_TextBounds);
		if (textHeight)
		{
			*textHeight = (m_TextBounds[3] - m_TextBounds[1]);
		}

		return w;
	}

	int VgRender::GetHandleForTexture(const Texture_ptr& handle)
	{
		if (m_TexMap.find(handle) != m_TexMap.end())
		{
			return m_TexMap[handle];
		}

		GLTexture* glTexture = GetTexture(handle);

		int newId = nvglCreateImageFromHandleGL3(m_VgContext, glTexture->Handle(), handle->GetDesc().Width, handle->GetDesc().Height, NVG_IMAGE_NODELETE);
		m_TexMap[handle] = newId;

		return newId;
	}

	void VgRender::DrawImage(const Texture_ptr& texture, float x, float y, float width, float height, float borderRadius, float alpha, float angle, bool centerX, bool centerY)
	{
		bool flip = !texture->GetDesc().IsRenderTarget;

		int handle = GetHandleForTexture(texture);
		NVGpaint paint;

		if (flip)
		{
			paint = nvgImagePattern(m_VgContext, x, y + height, width, -height, 0, handle, 1.0f);
		}
		else
		{
			paint = nvgImagePattern(m_VgContext, x, y, width, height, 0, handle, 1.0f);
		}

		nvgBeginPath(m_VgContext);
		if (borderRadius > 0)
		{
			nvgRoundedRect(m_VgContext, -(width / 2.0f), -(height / 2.0f), width, height, borderRadius);
		}
		else
		{
			nvgRect(m_VgContext, x, y, width, height);
		}
		nvgFillPaint(m_VgContext, paint);
		nvgFill(m_VgContext);
	}

	void VgRender::DrawImage(const Texture_ptr& texture, int x, int y, int width, int height, float borderRadius, float alpha, float angle)
	{
		DrawImage(texture, static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height), borderRadius, alpha, angle);
	}

	void VgRender::DrawImage(const Texture_ptr& texture, const Vector2i &location, const Vector2i &size, float borderRadius, float alpha, float angle, bool centerX, bool centerY)
	{
		DrawImage(texture, location.x, location.y, size.x, size.y, borderRadius, alpha, angle, centerX, centerY);
	}

	void VgRender::BeginClip(float x, float y, float width, float height)
	{
		nvgSave(m_VgContext);
		nvgIntersectScissor(m_VgContext, x, y, width, height);
	}

	void VgRender::BeginClip(const Vector2& pos, const Vector2& size)
	{
		BeginClip(pos.x, pos.y, size.x, size.y);
	}

	void VgRender::EndClip()
	{
		nvgRestore(m_VgContext);
	}

	void VgRender::DrawRect(float x, float y, float w, float h, Color color, bool isStroke, float strokeSize, float topLeft, float topRight, float bottomRight, float bottomLeft)
	{
		/* Shadow
		NVGpaint shadowPaint = nvgBoxGradient(_Context, x, y + 2, w, h, topLeft * 2, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
		nvgBeginPath(_Context);
		nvgRect(_Context, x - 10, y - 10, w + 20, h + 30);
		nvgRoundedRect(_Context, x, y, w, h, topLeft);
		nvgPathWinding(_Context, NVG_HOLE);
		nvgFillPaint(_Context, shadowPaint);
		nvgFill(_Context);*/

		nvgBeginPath(m_VgContext);
		if (bottomRight == topRight && bottomRight == bottomLeft && topLeft == bottomRight && topRight == bottomLeft)
		{
			nvgRoundedRect(m_VgContext, x, y, w, h, topLeft);
		}
		else
		{
			nvgRoundedRectVarying(m_VgContext, x, y, w, h, topLeft, topRight, bottomRight, bottomLeft);
		}
		NVGcolor c = nvgRGBA(color.r, color.g, color.b, color.a);
		if (isStroke)
		{
			nvgStrokeWidth(m_VgContext, strokeSize);
			nvgStrokeColor(m_VgContext, c);
			nvgStroke(m_VgContext);
		}
		else
		{
			nvgFillColor(m_VgContext, c);
			nvgFill(m_VgContext);
		}
	}

	void VgRender::DrawRect(int x, int y, int w, int h, Color color, bool isStroke, float strokeSize,
		float topLeft, float topRight, float bottomRight, float bottomLeft)
	{
		DrawRect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h), color, isStroke, strokeSize, topLeft, topRight, bottomRight, bottomLeft);
	}

	void VgRender::DrawOval(float x, float y, float w, float h, Color color, bool isStoke, float strokeSize)
	{
		nvgBeginPath(m_VgContext);

		nvgEllipse(m_VgContext, x, y, w, h);

		NVGcolor c = ColorToVg(color);

		if(isStoke) {
			nvgStrokeWidth(m_VgContext, strokeSize);
			nvgStrokeColor(m_VgContext, c);
			nvgStroke(m_VgContext);
		} else {
			nvgFillColor(m_VgContext, c);
			nvgFill(m_VgContext);
		}
	}

	void VgRender::DrawLine(float x0, float y0, float x1, float y1, float strength, Color startColor, Color endColor)
	{
		NVGpaint paint = nvgLinearGradient(m_VgContext, x0, y0, x1, y1, ColorToVg(startColor), ColorToVg(endColor));

		nvgBeginPath(m_VgContext);
		nvgMoveTo(m_VgContext, x0, y0);
		nvgLineTo(m_VgContext, x1, y1);

		nvgStrokeWidth(m_VgContext, strength);
		nvgStrokePaint(m_VgContext, paint);
		nvgStroke(m_VgContext);
	}

	void VgRender::DrawLine(float x0, float y0, float x1, float y1, float strength, Color color)
	{
		DrawLine(x0, y0, x1, y1, strength, color, color);
	}

	void VgRender::RB_RenderLine(float x1, float y1, float x2, float y2, float strokeWidth, Color color)
	{
		nvgBeginPath(m_VgContext);
		nvgMoveTo(m_VgContext, x1, y1);
		nvgLineTo(m_VgContext, x2, y2);
		nvgStrokeColor(m_VgContext, ColorToVg(color));
		nvgStrokeWidth(m_VgContext, strokeWidth);
		nvgStroke(m_VgContext);
	}

	void VgRender::RB_RenderGradientLR(float x, float y, float width, float height, Color leftColor, Color rightColor, float radius)
	{
		const NVGpaint paint = nvgLinearGradient(m_VgContext, x, y, x + width, y, ColorToVg(leftColor), ColorToVg(rightColor));
		nvgBeginPath(m_VgContext);
		nvgFillPaint(m_VgContext, paint);
		nvgRoundedRect(m_VgContext, x, y, width, height, radius);
		nvgClosePath(m_VgContext);
		nvgFill(m_VgContext);
	}

	void VgRender::RB_RenderGradientTB(float x, float y, float width, float height, Color topColor, Color bottomColor, float radius)
	{
		const NVGpaint paint = nvgLinearGradient(m_VgContext, x, y, x, y + height, ColorToVg(topColor), ColorToVg(bottomColor));
		nvgBeginPath(m_VgContext);
		nvgFillPaint(m_VgContext, paint);
		nvgRoundedRect(m_VgContext, x, y, width, height, radius);
		nvgClosePath(m_VgContext);
		nvgFill(m_VgContext);
	}

	void VgRender::RB_RenderBlock(const String & title, float x, float y, float width, float height, bool isSelected)
	{
		DrawRect(x, y, width, height, Color(37, 37, 38, 210)); //Background

		RB_RenderGradientLR(x, y, width, 25, Color(20, 100, 200), Color(4, 40, 80), 2.0f);

		DrawString(title, {x + 5, y + 6}, Color(181, 188, 188), 20, VgAlign::Left); //Title

		if (isSelected)
		{
			DrawRect(x, y, width, height, Color(82, 158, 207), true, 1.5f, 2.0f); //Outline
		}
		else
		{
			DrawRect(x, y, width, height, Color(100, 100, 100), true, 1.5f, 2.0f); //Outline
		}
	}

	void VgRender::RB_RenderSpline(float x1, float y1, float x2, float y2, int count, float strokeWidth)
	{
		NVGcolor color;
		color.a = 1.0f;
		color.r = 1.0f;
		color.g = 1.0f;
		color.b = 1.0f;

		static NVGcolor colors[4]{
			{ 1.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f }
		};

		float spsz = sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) * 0.5f;
		float spacing = 3.3f;
		float tbsw = spacing;

		if (x2 < x1)
		{
			std::swap(x1, x2);
			std::swap(y1, y2);
		}

		if (y2 < y1)
		{
			tbsw *= -1;
		}

		for (int i = 0; i < count; i++)
		{
			if (count > 1)
			{
				color = colors[i];
			}

			nvgBeginPath(m_VgContext);
			nvgMoveTo(m_VgContext, x1, y1 + (spacing * i));
			nvgBezierTo(m_VgContext, x1 + spsz - (tbsw * i), y1 + (spacing * i), x2 - spsz - (tbsw * i), y2 + (spacing * i), x2, y2 + (spacing * i));
			nvgStrokeColor(m_VgContext, color);
			nvgStrokeWidth(m_VgContext, strokeWidth);
			nvgStroke(m_VgContext);
		}
	}

	void VgRender::DrawCheck(float x, float y, Color color, float thickness)
	{
		nvgSave(m_VgContext);
		nvgBeginPath(m_VgContext);
		nvgTranslate(m_VgContext, x - 15, y - 16);
		nvgMoveTo(m_VgContext, 10, 17);
		nvgLineTo(m_VgContext, 13, 20);
		nvgLineTo(m_VgContext, 20, 13);
		nvgStrokeWidth(m_VgContext, thickness);
		nvgStrokeColor(m_VgContext, ColorToVg(color));
		nvgStroke(m_VgContext);
		nvgRestore(m_VgContext);
	}
}