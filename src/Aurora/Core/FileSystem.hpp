#pragma once

#include "Types.hpp"

namespace Aurora::FS
{
	std::vector<std::string> ReadLines(const Path &path);
	DataBlob LoadFile(const Path& path);
	bool IsDirectory(const Path &path);
	std::vector<Path> ListFiles(const Path &path, bool recursive);
	bool CreateDirectories(const Path &path);
	Path GetCurrentDir();
	bool FileExists(const Path &path);

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