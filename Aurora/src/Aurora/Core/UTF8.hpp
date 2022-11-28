#pragma once

#include "Aurora/Core/Library.hpp"
#include "Aurora/Logger/Logger.hpp"

#include <cstdint>
#include <string>

namespace Aurora
{
	AU_API std::u8string CodepointToUtf8(uint32_t codepoint);
}