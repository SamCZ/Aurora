#pragma once

#include "Aurora/Core/Color.hpp"
#include "../Material.hpp"

namespace Aurora
{
	class UIRenderer
	{
	private:
		Material_ptr m_Material;
		Matrix4 m_ProjectionMatrix;
	public:
		UIRenderer();

		void Begin(const Vector2i& size, const TEXTURE_FORMAT& textureFormat, const TEXTURE_FORMAT& depthFormat);
		void End();

		void FillRect(float x, float y, float w, float h, const Vector4& color, float radius = 0.0f);
		void DrawRect(float x, float y, float w, float h, const Vector4& color, float strokeSize, float radius = 0.0f);
		void DrawImage(float x, float y, float w, float h, float radius = 0.0f);

	private:
		enum class EVGCommand : uint8_t
		{
			MoveTo = 0,
			LineTo,
			BezierTo,
			Close,
			Winding
		};

		enum class EVGPointFlags : uint8_t
		{
			Corner = 1u << 0u,
			Left = 1u << 1u,
			Bevel = 1u << 2u,
			InnerBevel = 1u << 3u
		};
	};
}