#pragma once

#include <cstdint>

namespace Aurora
{
	typedef uint64_t TTypeID;

	constexpr TTypeID Hash_djb2(const char* str)
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

	TTypeID constexpr operator "" _HASH(const char* s, std::size_t) {
		return Hash_djb2(s);
	}
}