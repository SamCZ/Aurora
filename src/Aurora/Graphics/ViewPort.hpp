#pragma once

#include "Aurora/Core/Vector.hpp"

namespace Aurora
{
	struct FViewPort
	{
		int X;
		int Y;
		int Width;
		int Height;

		FViewPort() : X(0), Y(0), Width(0), Height(0) {}
		FViewPort(int x, int y, int w, int h) : X(x), Y(y), Width(w), Height(h) {}
		FViewPort(int w, int h) : X(0), Y(0), Width(w), Height(h) {}

		FViewPort(const Vector2& vec) : X(0), Y(0), Width(static_cast<int>(vec.x)), Height(static_cast<int>(vec.y)) {}
		FViewPort(const Vector2i& vec) : X(0), Y(0), Width(vec.x), Height(vec.y) {}
		FViewPort(const Vector2ui& vec) : X(0), Y(0), Width(static_cast<int>(vec.x)), Height(static_cast<int>(vec.y)) {}

		FViewPort(const Vector4& vec) : X(static_cast<int>(vec.x)), Y(static_cast<int>(vec.y)), Width(static_cast<int>(vec.z)), Height(static_cast<int>(vec.w)) {}
		FViewPort(const Vector4i& vec) : X(vec.x), Y(vec.y), Width(vec.z), Height(vec.w) {}
		FViewPort(const Vector4ui& vec) : X(static_cast<int>(vec.x)), Y(static_cast<int>(vec.y)), Width(static_cast<int>(vec.z)), Height(static_cast<int>(vec.w)) {}

		FViewPort(const Vector2i& pos, const Vector2i& size) : X(static_cast<int>(pos.x)), Y(static_cast<int>(pos.y)), Width(static_cast<int>(size.x)), Height(static_cast<int>(size.y)) {}

		FViewPort(const FViewPort& other) = default;

		bool operator==(const FViewPort& other) const
		{
			return X == other.X && Y == other.Y && Width == other.Width && Height == other.Height;
		}

		bool operator!=(const FViewPort& other) const
		{
			return !(operator==(other));
		}

		explicit operator glm::vec2() const
		{
			return {Width, Height};
		}

		explicit operator glm::vec4() const
		{
			return {X, Y, Width, Height};
		}
	};
}