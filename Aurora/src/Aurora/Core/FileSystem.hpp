#pragma once

#include "Types.hpp"

namespace Aurora::FS
{
	AU_API std::vector<std::string> ReadLines(const Path &path);
	AU_API DataBlob LoadFile(const Path& path);
	AU_API bool IsDirectory(const Path &path);
	AU_API std::vector<Path> ListFiles(const Path &path, bool recursive);
	AU_API bool CreateDirectories(const Path &path);
	AU_API Path GetCurrentDir();
	AU_API bool FileExists(const Path &path);

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