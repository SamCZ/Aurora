#pragma once

#include <string>
#include <sstream>
#include <fstream>

#include "Container.hpp"

namespace Aurora
{
	typedef std::string String;
	typedef std::wstring WString;

#define String_None String()
#define String_Empty String()

#define ToString(obj) std::to_string(obj)
#define Chr(charInt) String(1, (char)charInt)

	template<typename Out> static inline void SplitString(const String &s, char delim, Out result)
	{
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim))
		{
			*(result++) = item;
		}
	}

	static inline List<String> SplitString(const String &s, char delim)
	{
		List<String> elems;
		SplitString(s, delim, std::back_inserter(elems));
		return elems;
	}

	static inline bool StartsWith(const String& string, const String& expression)
	{
		/*
		if (string.rfind(expression, 0) == 0)
		{
			return true;
		}

		return false;*/

		if (expression.length() > string.length())
		{
			return false;
		}

		for (size_t i = 0; i < expression.length(); i++)
		{
			if (string[i] != expression[i])
			{
				return false;
			}
		}

		return true;
	}

	static inline bool StringContains(const String& str, char c)
	{
		for (size_t i = 0; i < str.length(); i++)
		{
			if (str[i] == c)
			{
				return true;
			}
		}

		return false;
	}

#define ToWString(str, var_name) wchar_t var_name[256]; MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, var_name, 256)
}