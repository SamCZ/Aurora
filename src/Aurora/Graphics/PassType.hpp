#pragma once

#include <cstdint>

namespace Aurora
{
	typedef uint8 PassType_t;

	enum class EPassType : PassType_t
	{
		Depth = 0,
		Ambient
	};

	constexpr char const* PassTypesToString[2] = {
			"Depth",
			"Ambient"
	};
}