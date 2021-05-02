#pragma once

#include "Font.hpp"
#include "Aurora/Core/Color.hpp"
#include "../Material.hpp"

namespace Aurora
{
	enum class ImageDrawMode : uint8_t
	{
		Simple = 0,
		Sliced,
		Tiled
	};

	struct SpriteBorder
	{
		float Left = 0;
		float Right = 0;
		float Top = 0;
		float Bottom = 0;

		inline SpriteBorder() = default;
		inline SpriteBorder(float left, float right, float top, float bottom) : Left(left), Right(right), Top(top), Bottom(bottom) { }
		inline SpriteBorder(float size) : Left(size), Right(size), Top(size), Bottom(size) { }
	};

	class UIRenderer
	{
	private:
		struct DrawArgs
		{
			float StrokeSize = 1.0f;
			float Radius = 0.0f;
			float Rotation = 0.0f;
			bool Fill = true;
			Vector4 Color = Vector4(1, 1, 1, 1);
			Vector4 Tint = Vector4(1, 1, 1, 1);
			RefCntAutoPtr<ITexture> Texture = RefCntAutoPtr<ITexture>(nullptr);
			Vector2 CustomUVs[4]{};
			bool EnabledCustomUVs = false;
			Material_ptr OverrideMaterial = nullptr;
		};
	private:
		Material_ptr m_Material;
		Material_ptr m_FontMaterial;
		Material_ptr m_LastMaterial;
		Matrix4 m_ProjectionMatrix;

		std::map<String, Font_ptr> m_Fonts;
		String m_CurrentFont;
	public:
		UIRenderer();
	public:
		void Begin(const Vector2i& size, const TEXTURE_FORMAT& textureFormat, const TEXTURE_FORMAT& depthFormat);
		void End();
	public:
		void FillRect(float x, float y, float w, float h, const Vector4& color, float radius = 0.0f, float rotation = 0.0f);
		void FillRect(const Vector2& position, const Vector2& size, const Vector4& color, float radius = 0.0f, float rotation = 0.0f);

		void DrawRect(float x, float y, float w, float h, const Vector4& color, float strokeSize, float radius = 0.0f, float rotation = 0.0f);
		void DrawRect(const Vector2& position, const Vector2& size, const Vector4& color, float strokeSize, float radius = 0.0f, float rotation = 0.0f);

		void DrawImage(float x, float y, float w, float h, const RefCntAutoPtr<ITexture>& texture, float radius = 0.0f, const ImageDrawMode& imageDrawMode = ImageDrawMode::Simple, const SpriteBorder& spriteBorder = SpriteBorder());
		void DrawImage(const Vector2& position, const Vector2& size, const RefCntAutoPtr<ITexture>& texture, float radius = 0.0f, const ImageDrawMode& imageDrawMode = ImageDrawMode::Simple, const SpriteBorder& spriteBorder = SpriteBorder());
		void SetImageEdgeDetection(bool enabled, int thickness = 3, const Vector4& edgeColor = Vector4(1.0));
	public:
		Font_ptr FindFont(const String& name);
		bool LoadFont(const String& name, const Path& path);
		void Text(const String& text, float x, float y, float fontSize, const Vector4& color, const String& fontName = "Default");
		void Text(const String& text, const Vector2& position, float fontSize, const Vector4& color, const String& fontName = "Default");
		Vector2 GetTextSize(const String& text, float fontSize, const String& fontName = "Default");
	private:
		void Draw(float x, float y, float w, float h, const DrawArgs& drawArgs);
	public:
		inline void SetFont(const String& fontName) { m_CurrentFont = fontName; }
		inline void SetDefaultFont() { m_CurrentFont = "Default"; }
		[[nodiscard]] inline const String& GetCurrentFont() const { return m_CurrentFont; }
	};
}