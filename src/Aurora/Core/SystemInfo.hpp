#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Aurora
{
	class SystemInfo
	{
	public:
		static std::size_t TotalVirtualMemory();
		static std::size_t UsedVirtualMemory();
		static std::size_t UsedVirtualMemoryByProcess();

		static std::size_t TotalPhysicalMemory();
		static std::size_t UsedPhysicalMemory();
		static std::size_t UsedPhysicalMemoryByProcess();

		static std::vector<std::string> PrintToString();
	};
}
