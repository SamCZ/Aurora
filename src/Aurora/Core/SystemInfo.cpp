#include "SystemInfo.hpp"

#if _WIN32
#include <Windows.h>
#include <psapi.h>
#elif defined(UNIX)
#include "sys/types.h"
#include "sys/sysinfo.h"
#endif

#include <sstream>
#include <iomanip>

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

	std::string FormatBytes(uint64_t bytes)
	{
		int marker = 1024; // Change to 1000 if required
		int decimal = 3; // Change as required
		int kiloBytes = marker; // One Kilobyte is 1024 bytes
		int megaBytes = marker * marker; // One MB is 1024 KB
		int gigaBytes = marker * marker * marker; // One GB is 1024 MB
		//uint64_t teraBytes = marker * marker * marker * marker; // One TB is 1024 GB

		std::stringstream ss;

		// return bytes if less than a KB
		if(bytes < kiloBytes)
		{
			ss << bytes << " Bytes";
		} // return KB if less than a MB
		else if(bytes < megaBytes)
		{
			ss << std::setprecision(decimal) << ((float)bytes / (float)kiloBytes) << " KB";
		}// return MB if less than a GB
		else if(bytes < gigaBytes)
		{
			ss << std::setprecision(decimal) << ((float)bytes / (float)megaBytes) << " MB";
		} // return GB if less than a TB
		else
		{
			ss << std::setprecision(decimal) << ((float)bytes / (float)gigaBytes) << " GB";
		}

		return ss.str();
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
			ss << "Total virtual memory: " << FormatBytes(totalVirtualMem);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}

		{
			ss << "Used virtual memory: " << FormatBytes(virtualMemUsed);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}

		{
			ss << "Used virtual memory (by process): " << FormatBytes(virtualMemUsedByMe);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}

		{
			ss << "Total physical Memory: " << FormatBytes(totalPhysMem);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}

		{
			ss << "Used physical Memory: " << FormatBytes(physMemUsed);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}

		{
			ss << "Used physical Memory (by process): " << FormatBytes(physMemUsedByMe);
			infos.push_back(ss.str());
			ss = std::stringstream();
		}
#else

#endif
		return infos;
	}
}