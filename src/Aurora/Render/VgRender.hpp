#pragma once

#include <map>
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Graphics/Color.hpp"
#include "Aurora/Graphics/Base/Texture.hpp"

struct NVGcontext;

namespace Aurora
{
	enum class VgAlign : uint8_t
	{
		Baseline,
		Bottom,
		Center,
		Left,
		Right,
		Top
	};

	class VgRender
	{
	private:
		NVGcontext* m_VgContext;
		std::map<Texture_ptr, int> m_TexMap;
		float m_TextBounds[4];
	public:
		VgRender();
		~VgRender();

		void Begin(const Vector2ui& screenSize, float devicePixelRatio);
		void End();

		bool LoadFont(const String& name, const Path& path);

		void DrawString(const String& text, const Vector2& pos, Color color, float fontSize, VgAlign xAlign = VgAlign::Left, VgAlign yAlign = VgAlign::Top);
		float GetTextSize(const String& text, float fontSize, const String& fontName, float* textHeight);

		NVGcontext* GetVgContext() { return m_VgContext; }

		void DrawImage(const Texture_ptr& texture, float x, float y, float width, float height, float borderRadius = 0.0f, float alpha = 1.0f, float angle = 0.0f, bool centerX = false, bool centerY = false);
		void DrawImage(const Texture_ptr& texture, int x, int y, int width, int height, float borderRadius = 0.0f, float alpha = 1.0f, float angle = 0.0f);
		void DrawImage(const Texture_ptr& texture, const Vector2i& location, const Vector2i& size, float borderRadius = 0.0f, float alpha = 1.0f, float angle = 0.0f, bool centerX = false, bool centerY = false);

		void BeginClip(float x, float y, float width, float height);
		void BeginClip(const Vector2& pos, const Vector2& size);
		void EndClip();

		void DrawRect(float x, float y, float w, float h, Color color, bool isStroke = false, float strokeSize = 0.0f, float topLeft = 0.0f, float topRight = 0.0f, float bottomRight = 0.0f, float bottomLeft = 0.0f);
		void DrawRect(int x, int y, int w, int h, Color color, bool isStroke = false, float strokeSize = 0.0f, float topLeft = 0.0f, float topRight = 0.0f, float bottomRight = 0.0f, float bottomLeft = 0.0f);
		void DrawOval(float x, float y, float w, float h, Color color, bool isStoke = false, float strokeSize = 0.0f);
		void DrawLine(float x0, float y0, float x1, float y1, float strength, Color startColor, Color endColor);
		void DrawLine(float x0, float y0, float x1, float y1, float strength, Color color);

		void RB_RenderLine(float x1, float y1, float x2, float y2, float strokeWidth, Color color);
		void RB_RenderGradientLR(float x, float y, float width, float height, Color leftColor, Color rightColor, float radius = 0.0f);
		void RB_RenderGradientTB(float x, float y, float width, float height, Color topColor, Color bottomColor, float radius = 0.0f);
		void RB_RenderBlock(const String& title, float x, float y, float width, float height, bool isSelected);
		void RB_RenderSpline(float x1, float y1, float x2, float y2, int count, float strokeWidth = 1.5f);

		void DrawCheck(float x, float y, Color color, float thickness);
	private:
		int GetHandleForTexture(const Texture_ptr& handle);
	};
}
