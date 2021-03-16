#pragma once

#include <DataBlobImpl.hpp>
#include <RefCntAutoPtr.hpp>

#include "Aurora/Core/Common.hpp"

using namespace Diligent;

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
		static RefCntAutoPtr<IDataBlob> ReadFileFromPackage(const Path& packageFile, const FileHeader& header);
		static std::map<Path, RefCntAutoPtr<IDataBlob>> ReadAllFilesFromPackage(const Path& packageFile);
	};
}
