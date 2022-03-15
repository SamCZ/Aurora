#pragma once

#include <cstdint>

namespace Aurora
{
	typedef uint8_t PassType_t;

	namespace Pass
	{
		constexpr PassType_t Ambient = 0;
		constexpr PassType_t Depth = 1;

		constexpr PassType_t Count = 2;
	}

	constexpr char const* PassTypesToString[2] = {
			"Ambient",
			"Depth"
	};
}