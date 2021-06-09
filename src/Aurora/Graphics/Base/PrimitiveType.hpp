#pragma once

namespace Aurora
{
	enum class PrimitiveType : uint8_t
	{
		PointList = 0,
		TriangleList,
		TriangleStrip,
		Patch1ControlPoint,
		Patch3ControlPoint
	};
}