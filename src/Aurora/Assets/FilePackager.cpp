#include "FilePackager.hpp"

#include <iostream>
#include <cstring>
#include <fstream>
#include <map>

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

			auto fileData = FS::LoadFile(filePath);

			header.Size = fileData.size();
			fileStream.write(reinterpret_cast<char*>(fileData.data()), fileData.size());
			currentOffset += fileData.size();
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

	DataBlob FilePackager::ReadFileFromPackage(const Path &packageFile, const FileHeader &header)
	{
		std::ifstream file(packageFile, std::ios::in | std::ios::binary);

		if (file.is_open()) {
			DataBlob fileData(header.CompressedSize > 0 ? header.CompressedSize : header.Size);

			file.seekg(header.Offset, std::ios::beg);
			file.read(reinterpret_cast<char*>(fileData.data()), fileData.size());

			file.close();
			return fileData;
		}

		return {};
	}

	std::map<Path, DataBlob> FilePackager::ReadAllFilesFromPackage(const Path &packageFile)
	{
		std::map<Path, DataBlob> fileMap;

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
				DataBlob fileData(header.CompressedSize > 0 ? header.CompressedSize : header.Size);
				file.seekg(header.Offset, std::ios::beg);
				file.read(reinterpret_cast<char*>(fileData.data()), fileData.size());
				fileMap[String(header.Filename)] = fileData;
			}

			file.close();
		}

		return fileMap;
	}
}