#pragma once

#include <iostream>
#include <filesystem>
#include <sstream>

#include <DataBlobImpl.hpp>
#include <RefCntAutoPtr.hpp>

#include "Container.hpp"
#include "String.hpp"
#include "Log.hpp"

#include "MemoryStream.hpp"
//#include <nlohmann/json.hpp>

using namespace Diligent;

namespace Aurora
{
    typedef std::filesystem::path Path;

    namespace FS
    {
        static inline Path GetCurrentDir();

        static Path CurrentDir = GetCurrentDir();

        static inline Path FixDevPath(const Path& path) {
            Path dirName = CurrentDir.filename();
            if(dirName == "cmake-build-debug" || dirName == "cmake-build-release") {
                return Path("../") / path;
            }

            if(dirName == "Client") {
                return Path("..") / Path("..") / path;
            }

            if(dirName == "Server") {
                return Path("..") / Path("..") / path;
            }

            if(dirName == "ConsoleGameUtils") {
                return Path("..") / Path("..") / path;
            }

            return path;
        }

        static inline List<String> ReadLines(const Path& path) {
            List<String> lines;

            std::ifstream stream(path.string());
            String line;

            if (stream.is_open())
            {
                while (std::getline(stream, line))
                {
                    lines.push_back(line);
                }
                stream.close();
            }

            return lines;
        }

        static inline List<String> ReadLines(RefCntAutoPtr<IDataBlob>& blob) {
            List<String> lines;

            if(blob == nullptr) {
                LogError("Cannot read lines !");
                return lines;
            }

            memstream stream(reinterpret_cast<uint8_t*>(blob->GetDataPtr()), blob->GetSize());
            String line;

            while (std::getline(stream, line))
            {
                lines.push_back(line);
            }

            //std::cout << String(blob->GetData(), blob->GetDataSize()) << std::endl;

            //std::stringstream stream(String(blob->GetData(), blob->GetDataSize()));

            /*String line;
            while (std::getline(stream, line))
            {
                lines.push_back(line);
            }*/

            return lines;
        }

        /*static inline Blob* LoadFile(const Path& path) {
            std::streampos size;
            char* memblock;

            std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
            if (file.is_open())
            {
                size = file.tellg();
                memblock = new char[size];
                file.seekg(0, std::ios::beg);
                file.read(memblock, size);
                file.close();

                return new Blob(memblock, size, path.filename().string());
            }

            return nullptr;
        }*/

        static inline bool IsDirectory(const Path& path) {
            return std::filesystem::is_directory(path);
        }

        static inline List<Path> ListFiles(const Path& path, bool recursive) {
            if(!IsDirectory(path)) {
                return {};
            }

            List<Path> files;

            if(recursive) {
                for(auto& file : std::filesystem::recursive_directory_iterator(path)) {
                    if(!IsDirectory(file)) {
                        files.push_back(file);
                    }
                }
            } else {
                for(auto& file : std::filesystem::directory_iterator(path)) {
                    files.push_back(file);
                }
            }

            return files;
        }

        static inline bool CreateDirectories(const Path& path) {
            return std::filesystem::create_directories(path);
        }

        static inline Path GetCurrentDir() {
            return std::filesystem::current_path();
        }

        static inline bool FileExists(const Path& path) {
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