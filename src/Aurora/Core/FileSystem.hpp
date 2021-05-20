#pragma once

#include <vector>
#include <filesystem>

#include "MemoryStream.hpp"

namespace Aurora
{
	using DataBlob = std::vector<uint8_t>;
	
	namespace FS
	{
		static inline Path GetCurrentDir();

		static Path CurrentDir = GetCurrentDir();

		static inline std::vector<String> ReadLines(const Path &path)
		{
			std::vector<String> lines;

			std::ifstream stream(path.string());
			String line;

			if (stream.is_open()) {
				while (std::getline(stream, line)) {
					lines.push_back(line);
				}
				stream.close();
			}

			return lines;
		}

		static inline std::vector<String> ReadLines(RefCntAutoPtr<IDataBlob> &blob)
		{
			std::vector<String> lines;

			if (blob == nullptr) {
				std::cerr << "Cannot read lines !" << std::endl;
				return lines;
			}

			memstream stream(reinterpret_cast<uint8_t *>(blob->GetDataPtr()), blob->GetSize());
			String line;

			while (std::getline(stream, line)) {
				lines.push_back(line);
			}

			return lines;
		}

		static inline DataBlob LoadFile(const Path& path)
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
				std::streamsize bytes = file.gcount();
				file.close();

				return fileData;
			}

			return {};
		}

		static inline bool IsDirectory(const Path &path)
		{
			return std::filesystem::is_directory(path);
		}

		static inline std::vector<Path> ListFiles(const Path &path, bool recursive)
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

		static inline bool CreateDirectories(const Path &path)
		{
			return std::filesystem::create_directories(path);
		}

		static inline Path GetCurrentDir()
		{
			return std::filesystem::current_path();
		}

		static inline bool FileExists(const Path &path)
		{
			return std::filesystem::exists(path);
		}

		/*static inline nlohmann::json LoadJson(const Path& file) {
			std::ifstream i(file);
			if (!i.is_open()) {
				return {};
			}
			nlohmann::json j;
			i >> j;
			i.close();
			return j;
		}

		static inline void SaveJson(nlohmann::json& js, const Path& file) {
			std::ofstream o(file);
			o << std::setw(4) << js << std::endl;
			o.close();
		}*/
	}
}
