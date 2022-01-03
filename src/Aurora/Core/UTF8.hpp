#pragma once

#include "Aurora/Logger/Logger.hpp"

#include <cstdint>
#include <string>

namespace Aurora
{
	std::u8string CodepointToUtf8(uint32_t codepoint);
}