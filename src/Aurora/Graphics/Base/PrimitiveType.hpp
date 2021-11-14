#pragma once

namespace Aurora
{
	enum class EPrimitiveType : uint8_t
	{
		PointList = 0,
		TriangleList,
		TriangleStrip,
		LineList,
		Patch1ControlPoint,
		Patch3ControlPoint
	};
}