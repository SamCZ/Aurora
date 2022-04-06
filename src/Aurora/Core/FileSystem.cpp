#include "FileSystem.hpp"
#include <fstream>
#include <iostream>

namespace Aurora::FS
{
	std::vector<std::string> ReadLines(const Path &path)
	{
		std::vector<std::string> lines;

		std::ifstream stream(path.string());
		std::string line;

		if (stream.is_open()) {
			while (std::getline(stream, line)) {
				lines.push_back(line);
			}
			stream.close();
		}

		return lines;
	}

	DataBlob LoadFile(const Path& path)
	{
		if(!std::filesystem::exists(path)) {
			std::cerr << "Cannot find file " << path << std::endl;
			exit(1);
		}

		std::streampos size;

		DataBlob fileData = {};

		std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
		if (file.is_open())
		{
			size = file.tellg();
			fileData.resize(size);
			file.seekg(0, std::ios::beg);
			file.read(reinterpret_cast<char *>(fileData.data()), size);
			file.close();

			return fileData;
		}

		return {};
	}

	bool IsDirectory(const Path &path)
	{
		return std::filesystem::is_directory(path);
	}

	std::vector<Path> ListFiles(const Path &path, bool recursive)
	{
		if (!IsDirectory(path)) {
			return {};
		}

		std::vector<Path> files;

		if (recursive) {
			for (auto &file : std::filesystem::recursive_directory_iterator(path)) {
				if (!IsDirectory(file)) {
					files.push_back(file);
				}
			}
		} else {
			for (auto &file : std::filesystem::directory_iterator(path)) {
				files.push_back(file);
			}
		}

		return files;
	}

	bool CreateDirectories(const Path &path)
	{
		return std::filesystem::create_directories(path);
	}

	Path GetCurrentDir()
	{
		return std::filesystem::current_path();
	}

	bool FileExists(const Path &path)
	{
		return std::filesystem::exists(path);
	}
}