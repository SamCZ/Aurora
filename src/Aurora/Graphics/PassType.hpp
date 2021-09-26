#pragma once

#include <cstdint>

namespace Aurora
{
	enum class EPassType : uint8_t
	{
		Depth = 0,
		Ambient
	};

	constexpr char const* PassTypesToString[2] = {
			"Depth",
			"Ambient"
	};
}