#pragma once

#include <map>
#include "Aurora/Core/Types.hpp"

namespace Aurora
{
	struct ABankHeader
	{
		char Filename[256];
		uint32_t Offset;
		uint32_t Size;
		uint32_t CompressedSize;
	};

	class AssetBank
	{
	private:

	public:
		static bool CreatePackage(const Path& baseFolderPath, const Path& outputFilePath);
		static std::map<Path, ABankHeader> ReadHeadersFromPackage(const Path& packageFile);
		static DataBlob ReadFileFromPackage(const Path& packageFile, const ABankHeader& header);
		static std::map<Path, DataBlob> ReadAllFilesFromPackage(const Path& packageFile);
	};
}
