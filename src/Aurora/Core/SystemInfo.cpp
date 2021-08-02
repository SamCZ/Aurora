#include "SystemInfo.hpp"

#if _WIN32
#include <Windows.h>
#include <psapi.h>
#elif defined(UNIX)
#include "sys/types.h"
#include "sys/sysinfo.h"
#endif

#include <sstream>

namespace Aurora
{
	std::size_t SystemInfo::TotalVirtualMemory()
	{
#if _WIN32
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		return memInfo.ullTotalPageFile;
#else
		return 0;
#endif
	}

	std::size_t SystemInfo::UsedVirtualMemory()
	{
#if _WIN32
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		return memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
#else
		return 0;
#endif
	}

	std::size_t SystemInfo::UsedVirtualMemoryByProcess()
	{
#if _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		return pmc.PrivateUsage;
#else
		return 0;
#endif
	}

	std::size_t SystemInfo::TotalPhysicalMemory()
	{
#if _WIN32
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		return memInfo.ullTotalPhys;
#else
		return 0;
#endif
	}

	std::size_t SystemInfo::UsedPhysicalMemory()
	{
#if _WIN32
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		return memInfo.ullTotalPhys - memInfo.ullAvailPhys;
#else
		return 0;
#endif
	}

	std::size_t SystemInfo::UsedPhysicalMemoryByProcess()
	{
#if _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		return pmc.WorkingSetSize;
#else
		return 0;
#endif
	}

	std::vector<std::string> SystemInfo::PrintToString()
	{
		std::vector<std::string> infos;
#if _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);

		DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
		DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
		SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

		DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
		DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
		SIZE_T physMemUsedByMe = pmc.WorkingSetSize;

		std::stringstream ss;

		{
			ss << "Total virtual memory: " << (totalVirtualMem / 1024 / 1024);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}

		{
			ss << "Used virtual memory: " << (virtualMemUsed / 1024 / 1024);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}

		{
			ss << "Used virtual memory (by process): " << (virtualMemUsedByMe / 1024 / 1024);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}

		{
			ss << "Total physical Memory: " << (totalPhysMem / 1024 / 1024);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}

		{
			ss << "Used physical Memory: " << (physMemUsed / 1024 / 1024);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}

		{
			ss << "Used physical Memory (by process): " << (physMemUsedByMe / 1024 / 1024);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}
#else

#endif
		return infos;
	}
}