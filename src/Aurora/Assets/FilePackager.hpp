#pragma once

#include "Aurora/Core/Common.hpp"
#include "Aurora/Core/DataBlob.hpp"
#include "Aurora/Core/FileSystem.hpp"

namespace Aurora
{
	struct FileHeader
	{
		char Filename[256];
		uint32_t Offset;
		uint32_t Size;
		uint32_t CompressedSize;
	};

	class FilePackager
	{
	public:
		static bool CreatePackage(const Path& baseFolderPath, const Path& outputFilePath);
		static std::map<Path, FileHeader> ReadHeadersFromPackage(const Path& packageFile);
		static DataBlob ReadFileFromPackage(const Path& packageFile, const FileHeader& header);
		static std::map<Path, DataBlob> ReadAllFilesFromPackage(const Path& packageFile);
	};
}
