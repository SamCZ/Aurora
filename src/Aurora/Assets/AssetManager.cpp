#include "AssetManager.hpp"

#include "Aurora/AuroraEngine.hpp"
#include "Aurora/Core/FileSystem.hpp"

#include <fstream>
#include <regex>

#define STB_IMAGE_IMPLEMENTATION
#include "Tools/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "Tools/stb_image_resize.h"

namespace Aurora
{
	AssetManager::AssetManager() = default;

	void AssetManager::LoadPackageFile(const Path& path)
	{
		auto map = FilePackager::ReadHeadersFromPackage(path);

		for(auto& it : map) {
			if(m_AssetPackageFiles.find(it.first) != m_AssetPackageFiles.end()) {
				std::cerr << "Found duplicated file: " << it.first << " in package:" << path << std::endl;
				continue;
			}

			m_AssetPackageFiles[it.first] = std::pair<Path, FileHeader>(path, it.second);

			Path fileFolder = it.first.parent_path();

			m_AssetPackageFolders[fileFolder].push_back(it.first);
		}
	}

	String AssetManager::LoadFileToString(const Path& path, bool* isFromAssetPackage)
	{
		auto blob = LoadFile(path, isFromAssetPackage);
		const char* str = reinterpret_cast<const char*>(blob.data());
		return std::string(str, str + blob.size());
	}

	DataBlob AssetManager::LoadFile(const Path& path, bool* isFromAssetPackage)
	{
		if(m_AssetPackageFiles.find(path) != m_AssetPackageFiles.end()) {
			if(isFromAssetPackage != nullptr) {
				*isFromAssetPackage = true;
			}

			auto& packageDesc = m_AssetPackageFiles[path];
			return FilePackager::ReadFileFromPackage(packageDesc.first, packageDesc.second);
		}

		if(isFromAssetPackage != nullptr) {
			*isFromAssetPackage = false;
		}

		return FS::LoadFile(path);
	}

	bool AssetManager::FileExists(const Path& path, bool* isFromAssetPackage) const
	{
		if(m_AssetPackageFiles.find(path) != m_AssetPackageFiles.end()) {
			if(isFromAssetPackage != nullptr) {
				*isFromAssetPackage = true;
			}
			return true;
		}

		if(isFromAssetPackage != nullptr) {
			*isFromAssetPackage = false;
		}

		return FS::FileExists(path);
	}

	unsigned int getMipLevelsNum(unsigned int width, unsigned int height)
	{
		unsigned int size = std::max<unsigned int>(width, height);
		unsigned int levelsNum = (unsigned int)(logf((float)size) / logf(2.0f)) + 1;

		return levelsNum;
	}

	Texture_ptr AssetManager::LoadTexture(const Path& path, const TextureLoadInfo& textureLoadInfo)
	{
		if(m_LoadedTextures.find(path) != m_LoadedTextures.end()) {
			return m_LoadedTextures[path];
		}

		auto fileData = LoadFile(path);

		if(fileData.empty()) {
			return nullptr;
		}

		Texture_ptr texture = nullptr;

		int width,height,channels_in_file;
		unsigned char *data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(fileData.data()), fileData.size(), &width, &height, &channels_in_file, STBI_rgb_alpha);
		if(!data) {
			AU_LOG_ERROR("Cannot load texture !", path.string());
			return nullptr;
		}

		TextureDesc textureDesc;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = getMipLevelsNum(width, height);
		textureDesc.ImageFormat = GraphicsFormat::RGBA8_UNORM;
		textureDesc.DebugName = path.string();
		texture = AuroraEngine::RenderDevice->CreateTexture(textureDesc, nullptr);

		for (unsigned int mipLevel = 0; mipLevel < textureDesc.MipLevels; mipLevel++)
		{
			AuroraEngine::RenderDevice->WriteTexture(texture, mipLevel, 0, data);

			if (mipLevel < textureDesc.MipLevels - 1u)
			{
				int newWidth = std::max<int>(1, width >> 1);
				int newHeight = std::max<int>(1, height >> 1);

				auto* resizedData = new uint8_t[newWidth * newHeight * STBI_rgb_alpha];
				stbir_resize_uint8(data, width, height, 0, resizedData, newWidth, newHeight, 0, STBI_rgb_alpha);

				stbi_image_free(data);
				data = resizedData;
				width = newWidth;
				height = newHeight;
			}
		}

		stbi_image_free(data);

		m_LoadedTextures[path] = texture;

		return texture;
	}

	Texture_ptr AssetManager::LoadTexture(const String& filename, const DataBlob& fileData, const TextureLoadInfo &textureLoadInfo)
	{
		if(fileData.empty()) {
			return nullptr;
		}

		Texture_ptr texture = nullptr;

		int width,height,channels_in_file;
		unsigned char *data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(fileData.data()), (int)fileData.size(), &width, &height, &channels_in_file, STBI_rgb_alpha);
		if(!data) {
			AU_LOG_ERROR("Cannot load texture !", filename);
			return nullptr;
		}

		TextureDesc textureDesc;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = getMipLevelsNum(width, height);
		textureDesc.ImageFormat = GraphicsFormat::RGBA8_UNORM;
		textureDesc.DebugName = "Loaded texture";
		texture = AuroraEngine::RenderDevice->CreateTexture(textureDesc, nullptr);

		for (unsigned int mipLevel = 0; mipLevel < textureDesc.MipLevels; mipLevel++)
		{
			AuroraEngine::RenderDevice->WriteTexture(texture, mipLevel, 0, data);

			if (mipLevel < textureDesc.MipLevels - 1u)
			{
				int newWidth = std::max<int>(1, width >> 1);
				int newHeight = std::max<int>(1, height >> 1);

				auto* resizedData = new uint8_t[newWidth * newHeight * STBI_rgb_alpha];
				stbir_resize_uint8(data, width, height, 0, resizedData, newWidth, newHeight, 0, STBI_rgb_alpha);

				stbi_image_free(data);
				data = resizedData;
				width = newWidth;
				height = newHeight;
			}
		}

		stbi_image_free(data);

		return texture;
	}

	bool AssetManager::LoadJson(const Path &path, nlohmann::json &json)
	{
		auto file = LoadFile(path);

		if(file.empty()) {
			return false;
		}

		auto* data = reinterpret_cast<uint8_t*>(file.data());
		auto dataLen = file.size();

		json = nlohmann::json::parse(data, data + dataLen);

		return true;
	}

	static std::map<String, EShaderType> ShaderFileTypeNames = { // NOLINT(cert-err58-cpp)
			{"vertex",   EShaderType::Vertex},
			{"fragment", EShaderType::Pixel},
			{"geometry", EShaderType::Geometry},
			{"hull",     EShaderType::Hull},
			{"domain",   EShaderType::Domain},
			{"compute",  EShaderType::Compute}
	};

	Shader_ptr AssetManager::LoadShaderFolder(const Path &path, const ShaderMacros& macros)
	{
		auto it = m_ShaderPrograms.find(path);

		if(it != m_ShaderPrograms.end()) {
			return m_ShaderPrograms[path];
		}

		ShaderProgramDesc shaderProgramDesc(path.string());

		for(auto& filePath: ListFiles(path)) {
			auto extension = filePath.extension().string();

			if(extension == ".disabled" || extension != ".glsl") continue;

			// Find shader type by filename

			auto filenameWithoutExtension = filePath.stem().string();
			auto shaderTypeIter = ShaderFileTypeNames.find(filenameWithoutExtension);

			if(shaderTypeIter == ShaderFileTypeNames.end()) {
				std::cerr << "Unknown shader type " << filenameWithoutExtension << std::endl;
				continue;
			}

			auto shaderType = ShaderFileTypeNames[filenameWithoutExtension];

			if(shaderType == EShaderType::Compute) {
				AU_LOG_WARNING("In shader pack ", path, " is present compute shader ! Skipping.");
				continue;
			}

			String shaderSource = LoadFileToString(filePath);

			/*{
				static const std::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");

				std::stringstream input;
				std::stringstream output;
				input << shaderSource;

				std::smatch matches;

				std::string line;
				while(std::getline(input,line))
				{
					if (std::regex_search(line, matches, re))
					{
						std::string include_file = matches[1];

						AU_LOG_INFO(include_file);
					} else {
						output << line;
					}
				}

				shaderSource = output.str();
			}*/

			shaderProgramDesc.AddShader(shaderType, shaderSource, macros);
		}

		auto shaderProgram = AuroraEngine::RenderDevice->CreateShaderProgram(shaderProgramDesc);

		if(shaderProgram == nullptr) {
			return nullptr;
		}

		m_ShaderPrograms[path] = shaderProgram;

		return shaderProgram;
	}

	Shader_ptr AssetManager::LoadComputeShader(const Path &path, const ShaderMacros& macros)
	{
		auto it = m_ShaderPrograms.find(path);

		if(it != m_ShaderPrograms.end()) {
			return m_ShaderPrograms[path];
		}

		ShaderProgramDesc shaderProgramDesc(path.string());
		shaderProgramDesc.AddShader(EShaderType::Compute, LoadFileToString(path), macros);

		auto shaderProgram = AuroraEngine::RenderDevice->CreateShaderProgram(shaderProgramDesc);

		if(shaderProgram == nullptr) {
			return nullptr;
		}

		m_ShaderPrograms[path] = shaderProgram;

		return shaderProgram;
	}

	/*const ShaderResourceObject_ptr &AssetManager::LoadShaderResource(const Path &path, const ShaderType& type)
	{
		if(m_ShaderResources.find(path) != m_ShaderResources.end()) {
			return m_ShaderResources[path];
		} else {
			m_ShaderResources[path] = ShaderResourceObject_ptr(new ShaderResourceObject(path, type));
			auto& res = m_ShaderResources[path];
			res->Load(false);
			return res;
		}
	}*/

	/*std::vector<ShaderResourceObject_ptr> AssetManager::LoadShaderResourceFolder(const Path &path)
	{
		std::vector<ShaderResourceObject_ptr> shaders;

		for(auto& filePath: ListFiles(path)) {
			auto extension = filePath.extension().string();

			if(extension == ".disabled" || extension != ".glsl") continue;

			// Find shader type by filename

			auto filenameWithoutExtension = filePath.stem().string();
			auto shaderTypeIter = ShaderFileTypeNames.find(filenameWithoutExtension);

			if(shaderTypeIter == ShaderFileTypeNames.end()) {
				std::cerr << "Unknown shader type " << filenameWithoutExtension << std::endl;
				continue;
			}

			auto shaderType = ShaderFileTypeNames[filenameWithoutExtension];

			// Load resource
			shaders.push_back(LoadShaderResource(filePath, shaderType));
		}

		return shaders;
	}*/

	std::vector<Path> AssetManager::ListFiles(const Path &path)
	{
		std::vector<Path> files;

		if(m_AssetPackageFolders.contains(path)) {
			for(const auto& file : m_AssetPackageFolders[path]) {
				files.push_back(file);
			}
		} else {
			for(auto& file: std::filesystem::directory_iterator(path)) {
				files.push_back(file.path());
			}
		}

		return files;
	}
}