#pragma once

#include <string>
#include <sstream>
#include <vector>

using String = std::basic_string<char>;

template<typename T>
inline String PointerToString(T* pointer)
{
	const void * address = static_cast<const void*>(pointer);
	std::stringstream ss;
	ss << address;
	return ss.str();
}

static std::vector<std::string> SplitString(const std::string& str, char delimiter)
{
	std::vector<std::string> list;
	std::stringstream buffer;

	for (char c : str) {
		if(c == delimiter) {
			if(buffer.tellp() != 0) {
				list.push_back(buffer.str());
				buffer.str("");
			}
		} else {
			buffer << c;
		}
	}

	if(buffer.tellp() != 0) {
		list.push_back(buffer.str());
	}

	return list;
}