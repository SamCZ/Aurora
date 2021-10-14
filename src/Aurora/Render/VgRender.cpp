#include "VgRender.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

#include "Aurora/Graphics/OpenGL/GL.hpp"
#include "Aurora/Graphics/NanoVG/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "Aurora/Graphics/NanoVG/nanovg_gl.h"

namespace Aurora
{
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
		auto fontData = GetEngine()->GetResourceManager()->LoadFile(path);

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

	void VgRender::DrawText(const String &text, const Vector2 &pos, Color color, int fontSize)
	{
		nvgFontSize(m_VgContext, static_cast<float>(fontSize));
		nvgFontFace(m_VgContext, "default");

		nvgFontBlur(m_VgContext, 0);
		nvgGlobalAlpha(m_VgContext, 1.0f);
		nvgTextAlign(m_VgContext, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFillColor(m_VgContext, nvgRGBA(color.r, color.g, color.b, color.a));
		nvgText(m_VgContext, pos.x, pos.y, text.c_str(), nullptr);
	}
}