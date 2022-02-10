#pragma once

#include <cstdint>

namespace Aurora
{
	typedef uint8 PassType_t;

	enum class EPassType : PassType_t
	{
		Ambient = 0,
		Depth = 1
	};

	constexpr char const* PassTypesToString[2] = {
			"Depth",
			"Ambient"
	};
}