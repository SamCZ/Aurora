#pragma once

#include "stb_truetype.h"

#include <RefCntAutoPtr.hpp>
#include <DataBlob.h>
#include <Texture.h>
#include <Aurora/Core/Common.hpp>
#include <Aurora/Core/Vector.hpp>

namespace Aurora
{
	using namespace Diligent;

	typedef uint32_t FontSize_t;
	typedef int32_t Codepoint_t;
	typedef std::vector<uint8_t> FontBitmap;

	struct BakedRect
	{
		float x;
		float y;
		float width;
		float height;
		Vector2 LeftTopUV;
		Vector2 RightBottomUV;
		RefCntAutoPtr<ITexture> Texture = RefCntAutoPtr<ITexture>(nullptr);
	};

	struct FontGlyph
	{
		int16_t X = 0;
		int16_t Y = 0;
		int16_t Width = 0;
		int16_t Height = 0;
		Vector2 LeftTopUV;
		Vector2 RightBottomUV;
		float xAdvance = 0;
		float xOff = 0;
		float yOff = 0;
		uint8_t PageIndex = 0;
	};

	struct FontBitmapPage
	{
		//float BaseLine = 0;
		FontBitmap Bitmap = {};
		RefCntAutoPtr<ITexture> Texture = RefCntAutoPtr<ITexture>(nullptr);
	};

	AU_CLASS(FontBitmapPageList)
	{
	private:
		FontSize_t m_FontSize;
		std::vector<FontGlyph> m_Glyphs;
		Codepoint_t m_FirstChar;
		Codepoint_t m_EndChar;

		std::vector<FontBitmapPage> m_FontBitmapPages;
	public:
		FontBitmapPageList(FontSize_t fontSize, Codepoint_t firstChar, Codepoint_t endChar);

		void Init(const stbtt_fontinfo& fontInfo);

		bool FindGlyph(Codepoint_t codepoint, FontGlyph& glyph);
		bool GetBakedRectForCodepoint(Codepoint_t codepoint, float x, float y, BakedRect& rect, float& xAdvance, float scale = 1.0f);
		bool GetBakedRectForCodepoint(const FontGlyph& glyph, float x, float y, BakedRect& rect, float& xAdvance, float scale = 1.0f);
		bool GetBitmapPage(const FontGlyph& glyph, FontBitmapPage& page);

		Vector2 GetStringSize(const String& string);
	};

	AU_CLASS(Font)
	{
	private:
		static std::vector<int> FontSizesDefs;
	private:
		String m_Name;
		RefCntAutoPtr<IDataBlob> FontData;
		stbtt_fontinfo m_FontInfo;
		std::map<FontSize_t, FontBitmapPageList_ptr> m_FontContainers;

		int m_FallbackFontSize;
	public:
		Font(String name, RefCntAutoPtr<IDataBlob>& data);
		FontBitmapPageList_ptr FindOrCreatePageList(FontSize_t fontSize);
	public:
		[[nodiscard]] FontSize_t FindSuitableSize(float fontSize) const;
	};
}
