#pragma once

#include <cstdint>

namespace Aurora::Editor
{
	enum class VarType : uint8_t
	{
		Unknown = 0,
		Char,
		Int,
		Float,
		Double,
		Vec2,
		Vec3,
		Vec4,

		IVec2,
		IVec3,
		IVec4
	};

	enum class ReflectEditorType : uint8_t
	{
		VarType
	};
}