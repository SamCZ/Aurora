#pragma once

#include <sstream>

namespace Aurora
{
	template <typename... RestArgsType>
	static std::string FormatString(const RestArgsType&... Args)
	{
		std::stringstream ss;

		(ss << ... << Args);

		return ss.str();
	}
}