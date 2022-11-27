#pragma once

#include "Hash.hpp"
#include <string>

#ifndef DS_PRETTY_FUNCTION
#   if defined _MSC_VER
#      define AU_PRETTY_FUNCTION __FUNCSIG__
#   else
#       if defined __GNUC__
#           define AU_PRETTY_FUNCTION __PRETTY_FUNCTION__
#       else
			static_assert(false, "OS still not supported");
#       endif
#   endif
#endif

typedef uint64_t TTypeID;

template<typename Type>
struct TypeIDCache
{
	static constexpr std::string_view ExtractTypeName(const char* name)
	{
		std::string_view view(name);

		auto first = view.find('<');
		auto last = view.find_last_of('>');

		std::string_view result = view.substr(first + 1, last - first - 1);
		if (result[result.length() - 1] == ' ')
		{
			return result.substr(0, result.length() - 1);
		}

		return result;
	}

	constexpr static const char* GetHashNameRaw()
	{ return AU_PRETTY_FUNCTION; }

	constexpr static std::string_view GetHashTypeName()
	{ return ExtractTypeName(GetHashNameRaw()); }

	constexpr static const StrHashID Value = HashDjb2(GetHashTypeName());
};
