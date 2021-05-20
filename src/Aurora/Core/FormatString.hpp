#pragma once

#include <sstream>

// this functions are modified from DiligentEngine

namespace Aurora
{
	template <typename SSType>
	inline void FormatStrSS(SSType& ss)
	{
	}

	template <typename SSType, typename ArgType>
	inline void FormatStrSS(SSType& ss, const ArgType& Arg)
	{
		ss << Arg;
	}

	template <typename SSType, typename FirstArgType, typename... RestArgsType>
	inline void FormatStrSS(SSType& ss, const FirstArgType& FirstArg, const RestArgsType&... RestArgs)
	{
		FormatStrSS(ss, FirstArg);
		FormatStrSS(ss, RestArgs...); // recursive call using pack expansion syntax
	}

	template <typename... RestArgsType>
	static std::string FormatString(const RestArgsType&... Args)
	{
		std::stringstream ss;
		FormatStrSS(ss, Args...);
		return ss.str();
	}
}