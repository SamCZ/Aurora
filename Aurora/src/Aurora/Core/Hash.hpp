#pragma once

#include "Types.hpp"

typedef uint64 StrHashID;

constexpr StrHashID HashDjb2(const char* str)
{
	unsigned long int hash = 5381;

	char c = *str;
	while(c != 0)
	{
		hash = ((hash << 5) + hash) + c;
		c = *(++str);
	}

	return hash;
}

constexpr StrHashID HashDjb2(const std::string_view& view)
{
	uint64 hash = 5381;

	for (const char c : view)
	{
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

StrHashID constexpr operator "" _HASH(const char* s, std::size_t) {
	return HashDjb2(s);
}