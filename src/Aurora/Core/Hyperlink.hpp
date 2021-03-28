#pragma once

#include <string>
inline void OpenLink(const std::string& url);

#if defined(_WIN32)
#   include <windows.h>
#   include <shellapi.h>

inline void OpenLink(const std::wstring& url)
{
	ShellExecute(nullptr, nullptr, url.c_str(), nullptr, nullptr , SW_SHOW );
}

inline void OpenLink(const std::string& url)
{
	OpenLink(std::wstring(url.begin(), url.end()));
}

#elif defined(__unix__)
#   include <cstdlib>

inline void OpenLink(const std::string& url)
{
    system(("open " + url).c_str());
}
#else
#   warning Unsupported OS
#endif