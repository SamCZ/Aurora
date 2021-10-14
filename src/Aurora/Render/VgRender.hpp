#pragma once

#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Graphics/Color.hpp"

struct NVGcontext;

namespace Aurora
{
	class VgRender
	{
	private:
		NVGcontext* m_VgContext;
	public:
		VgRender();
		~VgRender();

		void Begin(const Vector2ui& screenSize, float devicePixelRatio);
		void End();

		bool LoadFont(const String& name, const Path& path);

		void DrawText(const String& text, const Vector2& pos, Color color, int fontSize);
	};
}
