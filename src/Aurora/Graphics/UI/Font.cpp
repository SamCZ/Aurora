#include "Font.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <Aurora/AuroraEngine.hpp>

namespace Aurora
{
	std::vector<int> Font::FontSizesDefs = { // NOLINT(cert-err58-cpp)
			6, 12, 16, 20, 24, 42, 64, 82, 100, 112
	};

	Font::Font(String name, RefCntAutoPtr<IDataBlob> &data) : m_Name(std::move(name)), FontData(data), m_FontInfo(), m_FontContainers(), m_FallbackFontSize(16)
	{
		stbtt_InitFont(&m_FontInfo, reinterpret_cast<unsigned char *>(FontData->GetDataPtr()), 0);

		AU_LOG_INFO("Font loaded: ", name);
	}

	FontBitmapPageList_ptr Font::FindOrCreatePageList(FontSize_t fontSize)
	{
		auto it = m_FontContainers.find(fontSize);

		if(it != m_FontContainers.end()) {
			return it->second;
		}

		auto container = std::make_shared<FontBitmapPageList>(fontSize, 0x0020, 0x00FF);
		m_FontContainers[fontSize] = container;
		container->Init(m_FontInfo);

		AU_LOG_INFO("font created for size ", fontSize)

		return container;
	}

	FontSize_t Font::FindSuitableSize(float fontSize) const
	{
		for (int FontSizesDef : FontSizesDefs) {
			if(static_cast<float>(FontSizesDef) >= fontSize) {
				return FontSizesDef;
			}
		}

		return m_FallbackFontSize;
	}

	FontBitmapPageList::FontBitmapPageList(FontSize_t fontSize, Codepoint_t firstChar, Codepoint_t endChar)
	: m_FontSize(fontSize), m_Glyphs(), m_FirstChar(firstChar), m_EndChar(endChar)
	{
		m_Glyphs.resize((m_EndChar - m_FirstChar) + 1);


	}

	RefCntAutoPtr<ITexture> CreateTextureFromData(int w, int h, const unsigned char* data)
	{
		TextureDesc TexDesc;
		TexDesc.Name      = "FontTexture";
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

	void FontBitmapPageList::Init(const stbtt_fontinfo &fontInfo)
	{
		// Build bitmap pages

		const float scale = (m_FontSize > 0) ? stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(m_FontSize)) : stbtt_ScaleForMappingEmToPixels(&fontInfo, -static_cast<float>(m_FontSize));

		int OversampleH = 3;
		int OversampleV = 1;

		int total_surface = 0;

		// TODO: Implement multiple glyph pages

		for (Codepoint_t codepoint = m_FirstChar; codepoint <= m_EndChar; ++codepoint) {
			int x0, y0, x1, y1;
			const int glyph_index_in_font = stbtt_FindGlyphIndex(&fontInfo, codepoint);
			stbtt_GetGlyphBitmapBoxSubpixel(&fontInfo, glyph_index_in_font, scale * static_cast<float>(OversampleH), scale * static_cast<float>(OversampleV), 0, 0, &x0, &y0, &x1, &y1);

			int w = (stbrp_coord)(x1 - x0 + OversampleH - 1);
			int h = (stbrp_coord)(y1 - y0 + OversampleV - 1);

			total_surface += w * h;
		}

		const int surface_sqrt = (int)glm::sqrt((float)total_surface) + 1;
		const auto surface_sqrt_f = static_cast<float>(surface_sqrt);
		int textureSize = (surface_sqrt_f >= 4096 * 0.7f) ? 4096 : (surface_sqrt_f >= 2048 * 0.7f) ? 2048 : (surface_sqrt_f >= 1024 * 0.7f) ? 1024 : 512;

		{
			FontBitmap bitmap(textureSize * textureSize);
			std::fill(bitmap.begin(), bitmap.end(), 0);

			int x,y,bottom_y;
			x = y = 1;
			bottom_y = 1;

			float ipw = 1.0f / (float)textureSize, iph = 1.0f / (float)textureSize;

			for (Codepoint_t codepoint = m_FirstChar; codepoint <= m_EndChar; ++codepoint) {
				FontGlyph& glyph = m_Glyphs[codepoint - m_FirstChar];
				int advance, lsb, x0,y0,x1,y1,gw,gh;
				int g = stbtt_FindGlyphIndex(&fontInfo, codepoint);

				stbtt_GetGlyphHMetrics(&fontInfo, g, &advance, &lsb);
				stbtt_GetGlyphBitmapBox(&fontInfo, g, scale,scale, &x0,&y0,&x1,&y1);

				gw = x1-x0;
				gh = y1-y0;

				if (x + gw + 1 >= textureSize) {
					y = bottom_y, x = 1; // advance to next row
				}

				if (y + gh + 1 >= textureSize) {// check if it fits vertically AFTER potentially moving to next row
					return;// -i;
				}

				STBTT_assert(x+gw < textureSize);
				STBTT_assert(y+gh < textureSize);

				stbtt_MakeGlyphBitmap(&fontInfo, bitmap.data() + x + y * textureSize, gw, gh, textureSize, scale, scale, g);
				glyph.X = (stbtt_int16) x;
				glyph.Y = (stbtt_int16) y;
				glyph.Width = (stbtt_int16) gw;
				glyph.Height = (stbtt_int16) gh;
				glyph.xAdvance = glm::round(scale * static_cast<float>(advance));
				glyph.xOff     = (float) x0;
				glyph.yOff     = (float) y0;
				glyph.PageIndex = 0;

				glyph.LeftTopUV = Vector2(static_cast<float>(x) * ipw, static_cast<float>(y) * iph);
				glyph.RightBottomUV = Vector2((static_cast<float>(x + gw)) * ipw, (static_cast<float>(y + gh)) * iph);

				x = x + gw + 1;

				if (y+gh+1 > bottom_y) {
					bottom_y = y+gh+1;
				}
			}

			FontBitmapPage bitmapPage = {};
			bitmapPage.Bitmap = bitmap;
			//bitmapPage.BaseLine = static_cast<float>(bottom_y); // <- This was not the baseline
			bitmapPage.Texture = CreateTextureFromData(textureSize, textureSize, bitmap.data());
			m_FontBitmapPages.emplace_back(bitmapPage);
		}
	}

	bool FontBitmapPageList::FindGlyph(Codepoint_t codepoint, FontGlyph& glyph)
	{
		if(codepoint < m_FirstChar || codepoint > m_EndChar) {
			return false;
		}

		glyph = m_Glyphs[codepoint - m_FirstChar];

		return true;
	}

	bool FontBitmapPageList::GetBakedRectForCodepoint(Codepoint_t codepoint, float x, float y, BakedRect &rect, float &xAdvance, float scale)
	{
		FontGlyph glyph = {};

		if(!FindGlyph(codepoint, glyph)) {
			return false;
		}

		GetBakedRectForCodepoint(glyph, x, y, rect, xAdvance, scale);
		return true;
	}


	bool FontBitmapPageList::GetBakedRectForCodepoint(const FontGlyph &glyph, float x, float y, BakedRect &rect, float &xAdvance, float scale)
	{
		rect.x = STBTT_ifloor((x + glyph.xOff * scale) + 0.5f);
		rect.y = STBTT_ifloor((y + glyph.yOff * scale) + 0.5f);
		rect.width = static_cast<float>(glyph.Width) * scale;
		rect.height = static_cast<float>(glyph.Height) * scale;
		rect.LeftTopUV = glyph.LeftTopUV;
		rect.RightBottomUV = glyph.RightBottomUV;
		rect.Texture = m_FontBitmapPages[glyph.PageIndex].Texture;

		xAdvance = glyph.xAdvance * scale;
		return true;
	}

	bool FontBitmapPageList::GetBitmapPage(const FontGlyph &glyph, FontBitmapPage &page)
	{
		page = m_FontBitmapPages[glyph.PageIndex];
		return true;
	}

	Vector2 FontBitmapPageList::GetStringSize(const String &string)
	{
		Vector2 size = {0, 0};

		return size;
	}
}