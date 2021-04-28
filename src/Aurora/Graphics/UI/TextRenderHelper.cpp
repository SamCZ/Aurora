#include "TextRenderHelper.hpp"

namespace Aurora
{
	TextRenderHelper::TextRenderHelper(UIRenderer* uiRenderer)
			: m_UIRenderer(uiRenderer), m_Dims({0, 0}), m_DimsComputed(false), m_MaxLeftX(0), m_MaxLeftY(0), m_MaxRightX(0), m_ActiveFont("Default") { }

	TextRenderHelper::~TextRenderHelper()
	{
		for(FTextRenderData& renderData : m_LineData) {
			delete renderData.RightData;
		}
	}

	void TextRenderHelper::AddTextLine(const String &text, float fontSize, const Vector4 &color)
	{
		auto size = m_UIRenderer->GetTextSize(text, fontSize, m_ActiveFont);

		FTextRenderData data;
		data.Text = text;
		data.X = 0;
		data.Y = m_MaxLeftY;
		data.Width = size.x;
		data.FontSize = fontSize;
		data.Color = color;
		data.Font = m_ActiveFont;

		m_MaxLeftX = glm::max(m_MaxLeftX, data.Width);
		m_MaxLeftY += size.y;

		m_LineData.emplace_back(data);
	}

	void TextRenderHelper::AddSameLine(const String &text, float fontSize, const Vector4 &color)
	{
		if(m_LineData.empty()) {
			return;
		}

		FTextRenderData& leftData = m_LineData[m_LineData.size() - 1];

		if(leftData.RightData != nullptr) {
			return;
		}

		auto size = m_UIRenderer->GetTextSize(text, fontSize, m_ActiveFont);

		FTextRenderData data;
		data.Text = text;
		data.X = -1;
		data.Y = leftData.Y;
		data.Width = size.x;
		data.FontSize = fontSize;
		data.Color = color;
		data.Font = m_ActiveFont;

		m_MaxRightX = glm::max(m_MaxRightX, data.Width);

		leftData.RightData = new FTextRenderData(data);
	}

	void TextRenderHelper::ComputeDims()
	{
		for(FTextRenderData& renderData : m_LineData) {
			if(renderData.RightData == nullptr) {
				continue;
			}

			m_MaxLeftX += 10;

			break;
		}

		for(FTextRenderData& renderData : m_LineData) {
			if(renderData.RightData == nullptr) {
				continue;
			}

			renderData.RightData->X = m_MaxLeftX;
		}

		m_Dims.x = m_MaxLeftX + m_MaxRightX;
		m_Dims.y = m_MaxLeftY;
		m_DimsComputed = true;
	}

	Vector2 TextRenderHelper::GetDims() const
	{
		return m_Dims;
	}

	void TextRenderHelper::Render(float x, float y)
	{
		if(!m_DimsComputed) {
			ComputeDims();
		}

		String lastFont = m_UIRenderer->GetCurrentFont();

		for(const FTextRenderData& renderData : m_LineData) {
			m_UIRenderer->Text(renderData.Text, x + renderData.X, y + renderData.Y, renderData.FontSize, renderData.Color, renderData.Font);

			if(renderData.RightData != nullptr) {
				m_UIRenderer->Text(renderData.RightData->Text, x + renderData.RightData->X, y + renderData.RightData->Y, renderData.RightData->FontSize, renderData.RightData->Color, renderData.RightData->Font);
			}
		}

		m_UIRenderer->SetFont(lastFont);
	}

	void TextRenderHelper::Render(const Vector2 &pos)
	{
		Render(pos.x, pos.y);
	}

	void TextRenderHelper::Reset()
	{
		m_LineData.clear();
		m_Dims = Vector2i(0, 0);

		m_MaxLeftX = 0;
		m_MaxRightX = 0;
		m_MaxLeftY = 0;

		m_DimsComputed = false;
	}

	void TextRenderHelper::SetFont(const String& fontName)
	{
		m_ActiveFont = fontName;
	}
}