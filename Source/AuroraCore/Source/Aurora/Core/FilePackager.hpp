#pragma once

#include <cstdint>
#include "FileSystem.hpp"
#include <DataBlobImpl.hpp>
#include <RefCntAutoPtr.hpp>

namespace Aurora
{
    struct FileHeader
    {
        char Filename[256];
        uint32_t Offset;
        uint32_t Size;
        uint32_t CompressedSize;
    };

    class FFilePackager
    {
    public:
        static bool CreatePackage(const Path& baseFolderPath, const Path& outputFilePath);
        static Map<Path, FileHeader> ReadHeadersFromPackage(const Path& packageFile);
        static RefCntAutoPtr<IDataBlob> ReadFileFromPackage(const Path& packageFile, const FileHeader& header);
        static Map<Path, RefCntAutoPtr<IDataBlob>> ReadAllFilesFromPackage(const Path& packageFile);
    };
}