#include "FilePackager.hpp"

#include <iostream>
#include <cstring>
#include <map>

#include <BasicFileSystem.hpp>
#include <BasicFileStream.hpp>

#include "Aurora/Core/FileSystem.hpp"

namespace Aurora
{
	Path StripFirstDir(Path p)
	{
		p = p.relative_path();
		if (p.empty()) return {};

		if(*p.begin() != "..") return p;

		return p.lexically_relative(*p.begin());
	}

	bool FilePackager::CreatePackage(const Path &baseFolderPath, const Path& outputFilePath)
	{
		std::fstream fileStream;
		fileStream.open(outputFilePath, std::ios::out | std::ios::binary);

		std::vector<Path> files = FS::ListFiles(baseFolderPath, true);
		int32_t fileCount = files.size();
		int32_t headerBeginPos = sizeof(int32_t);
		int32_t currentOffset = headerBeginPos + (sizeof(FileHeader) * fileCount);
		auto* headers = new FileHeader[fileCount];

		fileStream.write(reinterpret_cast<char*>(&fileCount), sizeof(int32_t));

		fileStream.seekp(currentOffset, std::ios::beg);
		for (int i = 0; i < files.size(); ++i) {
			Path& filePath = files[i];

			FileHeader& header = headers[i];
			header.CompressedSize = 0;
			header.Offset = currentOffset;
			strcpy(header.Filename, StripFirstDir(filePath).string().c_str());

			// TODO: Compression
			// https://github.com/lz4/lz4

			RefCntAutoPtr<BasicFileStream> pFileStream(MakeNewRCObj<BasicFileStream>()((const Char*)filePath.string().c_str(), EFileAccessMode::Read));
			if (!pFileStream->IsValid())
				LOG_ERROR_AND_THROW("Failed to open image file \"", filePath, '\"');

			RefCntAutoPtr<IDataBlob> pFileData(MakeNewRCObj<DataBlobImpl>()(0));
			pFileStream->ReadBlob(pFileData);

			header.Size = pFileData->GetSize();
			fileStream.write(reinterpret_cast<char*>(pFileData->GetDataPtr()), pFileData->GetSize());
			currentOffset += pFileData->GetSize();
		}

		fileStream.seekp(headerBeginPos, std::ios::beg);
		fileStream.write(reinterpret_cast<char*>(headers), sizeof(FileHeader) * fileCount);

		fileStream.close();
		return true;
	}

	std::map<Path, FileHeader> FilePackager::ReadHeadersFromPackage(const Path &packageFile)
	{
		std::map<Path, FileHeader> fileHeaders;

		std::ifstream file(packageFile, std::ios::in | std::ios::binary);

		if (file.is_open()) {
			int32_t fileCount;
			file.read(reinterpret_cast<char*>(&fileCount), sizeof(int32_t));

			for (int i = 0; i < fileCount; ++i) {
				FileHeader header = {};
				file.read(reinterpret_cast<char*>(&header), sizeof(header));
				fileHeaders[String(header.Filename)] = header;
			}

			file.close();
		}

		return fileHeaders;
	}

	RefCntAutoPtr<IDataBlob> FilePackager::ReadFileFromPackage(const Path &packageFile, const FileHeader &header)
	{
		std::ifstream file(packageFile, std::ios::in | std::ios::binary);

		if (file.is_open()) {
			RefCntAutoPtr<IDataBlob> pFileData(MakeNewRCObj<DataBlobImpl>()(header.CompressedSize > 0 ? header.CompressedSize : header.Size));

			file.seekg(header.Offset, std::ios::beg);
			file.read(reinterpret_cast<char*>(pFileData->GetDataPtr()), pFileData->GetSize());

			file.close();
			return pFileData;
		}

		return RefCntAutoPtr<IDataBlob>(nullptr);
	}

	std::map<Path, RefCntAutoPtr<IDataBlob>> FilePackager::ReadAllFilesFromPackage(const Path &packageFile)
	{
		std::map<Path, RefCntAutoPtr<IDataBlob>> fileMap;

		std::ifstream file(packageFile, std::ios::in | std::ios::binary);

		if (file.is_open()) {
			std::vector<FileHeader> headers;
			int32_t fileCount;
			file.read(reinterpret_cast<char*>(&fileCount), sizeof(int32_t));

			for (int i = 0; i < fileCount; ++i) {
				FileHeader header = {};
				file.read(reinterpret_cast<char*>(&header), sizeof(header));
				headers.emplace_back(header);
			}

			for(const FileHeader& header : headers) {
				RefCntAutoPtr<IDataBlob> pFileData(MakeNewRCObj<DataBlobImpl>()(header.CompressedSize > 0 ? header.CompressedSize : header.Size));
				file.seekg(header.Offset, std::ios::beg);
				file.read(reinterpret_cast<char*>(pFileData->GetDataPtr()), pFileData->GetSize());
				fileMap[String(header.Filename)] = pFileData;
			}

			file.close();
		}

		return fileMap;
	}
}