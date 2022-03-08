#pragma once

#include <cstdint>

namespace Aurora
{
	typedef uint8_t PassType_t;

	namespace Pass
	{
		static constexpr PassType_t Ambient = 0;
		static constexpr PassType_t Depth = 1;
	}

	constexpr char const* PassTypesToString[2] = {
			"Ambient",
			"Depth"
	};
}