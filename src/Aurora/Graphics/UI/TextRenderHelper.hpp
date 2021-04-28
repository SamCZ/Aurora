#pragma once

#include <Aurora/Core/Common.hpp>
#include <Aurora/Core/Vector.hpp>

#include "UIRenderer.hpp"

namespace Aurora
{
	class TextRenderHelper
	{
	private:
		struct FTextRenderData
		{
			String Text;
			float X = 0;
			float Y = 0;
			float Width = 0;
			float FontSize = 0;
			Vector4 Color = {};
			String Font;
			FTextRenderData* RightData = nullptr;
		};
	private:
		UIRenderer* m_UIRenderer;
		std::vector<FTextRenderData> m_LineData;
		Vector2 m_Dims;
		float m_MaxLeftX;
		float m_MaxRightX;
		float m_MaxLeftY;
		bool m_DimsComputed;
		String m_ActiveFont;
	public:
		explicit TextRenderHelper(UIRenderer* uiRenderer);
		~TextRenderHelper();

		void AddTextLine(const String& text, float fontSize, const Vector4& color = Vector4(1.0f));
		void AddSameLine(const String& text, float fontSize, const Vector4& color = Vector4(1.0f));

		void ComputeDims();
		[[nodiscard]] Vector2 GetDims() const;

		void Render(float x, float y);
		void Render(const Vector2& pos);

		void Reset();

		void SetFont(const String& fontName);
	};
}
