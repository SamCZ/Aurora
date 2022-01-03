#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

using String = std::basic_string<char>;

template<typename T>
inline String PointerToString(T* pointer)
{
	const void * address = static_cast<const void*>(pointer);
	std::stringstream ss;
	ss << address;
	return ss.str();
}

std::vector<std::string> SplitString(const std::string& str, char delimiter);
std::string FormatBytes(uint64_t bytes);