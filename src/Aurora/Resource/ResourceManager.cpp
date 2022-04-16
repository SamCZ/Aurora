#include "ResourceManager.hpp"

#include <fstream>
#include <regex>
#include "Aurora/Core/FileSystem.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "MaterialLoader.hpp"
#include "AssimpModelLoader.hpp"

#include "Aurora/Core/Profiler.hpp"
#include "Aurora/Core/UUID.hpp"
#include "Aurora/App/AppContext.hpp"
#include "Aurora/Graphics/RenderManager.hpp"

namespace Aurora
{
	void FileTreeContainer::OnTreeChanged(EFileAction action, const Path &path, const Path& prevPath)
	{
		Path relativePath = std::filesystem::relative(path, Root.parent_path());

		switch (action)
		{
			case EFileAction::Added:
				Tree->AddFile(path);
				AU_LOG_INFO("File Added ", relativePath.string());
				break;
			case EFileAction::Renamed:
				Tree->RenameFile(path, prevPath);
				AU_LOG_INFO("File Renamed from ", prevPath.string(), " to ", path.string());
				break;
			case EFileAction::Modified:
				AU_LOG_INFO("File Modified ", relativePath.string());
				break;
			case EFileAction::Removed:
				Tree->RemoveFile(path);
				AU_LOG_INFO("File Removed ", relativePath.string());
				break;
			default: break;
		}
	}

	ResourceManager::ResourceManager(IRenderDevice* renderDevice) : m_RenderDevice(renderDevice)
	{

	}

	ResourceManager::~ResourceManager()
	{
		for (const auto& [path, treeCont] : m_FileTrees)
		{
			delete treeCont;
		}
	}

	void ResourceManager::Update()
	{
		for (const auto& [path, treeCont] : m_FileTrees)
		{
			if (treeCont->FileWatcher)
				treeCont->FileWatcher->Update();
		}
	}

	void ResourceManager::AddFileSearchPath(const String &path)
	{
		m_FileSearchPaths.emplace_back(path);

		if (AppContext::IsEditorMode())
		{
			CreateFileTree(Path(path) / "Assets", true);
		}
	}

	FileTree* ResourceManager::CreateFileTree(const Path &path, bool watch)
	{
		auto it = m_FileTrees.find(path);

		if (it != m_FileTrees.end())
		{
			return it->second->Tree;
		}

		auto* container = new FileTreeContainer();
		container->Root = path;
		container->Tree = new FileTree(path);
		container->FileWatcher = nullptr;

		if (watch)
		{
			container->FileWatcher = new FileWatcher(path);
			container->FileWatcher->GetEmitter().Bind(container, &FileTreeContainer::OnTreeChanged);
		}

		m_FileTrees[path] = container;

		return container->Tree;
	}

	FileTree* ResourceManager::GetFileTree(const Path& path)
	{
		auto it = m_FileTrees.find(path);

		if (it != m_FileTrees.end())
		{
			return it->second->Tree;
		}

		return nullptr;
	}

	void ResourceManager::LoadPackageFile(const Path& path)
	{
		auto map = AssetBank::ReadHeadersFromPackage(path);

		for(auto& it : map) {
			if(m_AssetPackageFiles.find(it.first) != m_AssetPackageFiles.end()) {
				std::cerr << "Found duplicated file: " << it.first << " in package:" << path << std::endl;
				continue;
			}

			m_AssetPackageFiles[it.first] = std::pair<Path, ABankHeader>(path, it.second);

			Path fileFolder = it.first.parent_path();

			m_AssetPackageFolders[fileFolder].push_back(it.first);
		}
	}

	String ResourceManager::LoadFileToString(const Path& path, bool* isFromAssetPackage)
	{
		auto blob = LoadFile(path, isFromAssetPackage);
		const char* str = reinterpret_cast<const char*>(blob.data());
		return std::string(str, str + blob.size());
	}

	DataBlob ResourceManager::LoadFile(const Path& path, bool* isFromAssetPackage)
	{
		if(m_AssetPackageFiles.find(path) != m_AssetPackageFiles.end()) {
			if(isFromAssetPackage != nullptr) {
				*isFromAssetPackage = true;
			}

			auto& packageDesc = m_AssetPackageFiles[path];
			return AssetBank::ReadFileFromPackage(packageDesc.first, packageDesc.second);
		}

		if(isFromAssetPackage != nullptr) {
			*isFromAssetPackage = false;
		}

		for (const auto &item : m_FileSearchPaths)
		{
			Path newPath = item / path;

			if(FS::FileExists(newPath))
			{
				return FS::LoadFile(newPath);
			}
		}

		return FS::LoadFile(path);
	}

	bool ResourceManager::FileExists(const Path& path, bool* isFromAssetPackage) const
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

		for (const auto &item : m_FileSearchPaths)
		{
			Path newPath = item / path;

			if(FS::FileExists(newPath))
			{
				return true;
			}
		}

		return FS::FileExists(path);
	}

	bool ResourceManager::GetRealPath(const Path& path, Path& path_out) const
	{
		for (const auto &item : m_FileSearchPaths)
		{
			Path newPath = item / path;

			if(FS::FileExists(newPath))
			{
				path_out = newPath;
				return true;
			}
		}

		if (FS::FileExists(path))
		{
			path_out = path;
			return true;
		}

		return false;
	}

	String ResourceManager::ReadShaderSource(const Path &path, std::vector<Path>& alreadyIncluded)
	{
		String shaderSource = LoadFileToString(path);
		{
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
					if(path.filename() == "common.h")
					{
						continue;
					}

					std::string include_file = matches[1];
					Path includePath = path.parent_path() / include_file;

					// If path contains / at first character, we just absolute path
					if(include_file.starts_with('/'))
					{
						includePath = include_file.substr(1);
					}

					if(std::find(alreadyIncluded.begin(), alreadyIncluded.end(), includePath.filename()) != alreadyIncluded.end())
					{
						continue;
					}

					if(!FileExists(includePath))
					{
						AU_LOG_FATAL("File " , includePath.string(), " not found !");
					}

					//std::cout << "including " << includePath.string() << " (" << includePath.filename() << ") " << " in " << path.string() << std::endl;

					alreadyIncluded.push_back(includePath.filename());
					output << ReadShaderSource(includePath, alreadyIncluded) << std::endl;
				} else {
					output << line << std::endl;
				}
			}

			shaderSource = output.str();
		}

		return shaderSource;
	}

	Shader_ptr ResourceManager::LoadComputeShader(const Path &path, const ShaderMacros &macros)
	{
		auto it = m_ShaderPrograms.find(path);

		if(it != m_ShaderPrograms.end()) {
			return m_ShaderPrograms[path];
		}

		String shaderSource = ReadShaderSource(path);
		ShaderProgramDesc shaderProgramDesc(path.string());
		shaderProgramDesc.AddShader(EShaderType::Compute, shaderSource, macros);

		auto shaderProgram = m_RenderDevice->CreateShaderProgram(shaderProgramDesc);

		if(shaderProgram == nullptr) {
			return nullptr;
		}

		m_ShaderPrograms[path] = shaderProgram;

		return shaderProgram;
	}

	ShaderProgramDesc ResourceManager::CreateShaderProgramDesc(const String& name, const std::map<EShaderType, Path>& shaderTypesPaths, const ShaderMacros &macros)
	{
		ShaderProgramDesc shaderProgramDesc(name);

		if(shaderTypesPaths.size() == 1 && shaderTypesPaths.begin()->first == EShaderType::Compute)
		{
			shaderProgramDesc.AddShader(EShaderType::Compute, ReadShaderSource(shaderTypesPaths.begin()->second), macros);
			return shaderProgramDesc;
		}

		for (const auto &item : shaderTypesPaths)
		{
			if(item.first == EShaderType::Compute && shaderTypesPaths.size() > 1)
			{
				AU_LOG_FATAL("You cannot mix compute shader with others !");
			}

			Path filePath = item.second;

			if(!FileExists(filePath))
			{
				AU_LOG_FATAL("File " , filePath.string(), " not found !");
			}

			String shaderSource = ReadShaderSource(filePath);
			shaderProgramDesc.AddShader(item.first, shaderSource, macros);
		}

		return shaderProgramDesc;
	}

	Shader_ptr ResourceManager::LoadShader(const String& name, const std::map<EShaderType, Path>& shaderTypesPaths, const ShaderMacros &macros)
	{
		// TODO: Simplify this with CreateShaderProgramDesc method

		/*std::stringstream ss;

		for (const auto &item : shaderTypesPaths)
		{
			ss << item.second.string() << ";";
		}

		String multipath = ss.str();

		auto it = m_ShaderPrograms.find(multipath);

		if(it != m_ShaderPrograms.end()) {
			return m_ShaderPrograms[multipath];
		}*/

		ShaderProgramDesc shaderProgramDesc(name);

		for (const auto &item : shaderTypesPaths)
		{
			if(item.first == EShaderType::Compute && shaderTypesPaths.size() > 1)
			{
				AU_LOG_FATAL("You cannot mix compute shader with others !");
			}

			Path filePath = item.second;

			if(!FileExists(filePath))
			{
				AU_LOG_FATAL("File " , filePath.string(), " not found !");
			}

			String shaderSource = ReadShaderSource(filePath);
			shaderProgramDesc.AddShader(item.first, shaderSource, macros);
		}

		auto shaderProgram = m_RenderDevice->CreateShaderProgram(shaderProgramDesc);

		if(shaderProgram == nullptr) {
			return nullptr;
		}

		//m_ShaderPrograms[multipath] = shaderProgram;

		AU_LOG_INFO("Loaded shader: ", name);

		return shaderProgram;
	}

	bool ResourceManager::LoadJson(const Path &path, nlohmann::json &json)
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

	Texture_ptr ResourceManager::LoadTexture(const Path &path, const TextureLoadDesc& loadDesc)
	{
		if (!loadDesc.DoNotCache && m_LoadedTextures.find(path) != m_LoadedTextures.end()) {
			return m_LoadedTextures[path];
		}

		bool fromAssetPackage = false;
		auto fileData = LoadFile(path, &fromAssetPackage);

		if (fileData.empty()) {
			return nullptr;
		}

		Texture_ptr texture = nullptr;

		int width,height,channels_in_file;
		unsigned char *data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(fileData.data()), fileData.size(), &width, &height, &channels_in_file, STBI_rgb_alpha);
		if (!data) {
			AU_LOG_ERROR("Cannot load texture !", path.string());
			return nullptr;
		}

		channels_in_file = 4;

		GraphicsFormat format = GraphicsFormat::RGBA8_UNORM;

		// stbi_load will return you actual channels_in_file even if you request something
		// YES if will change the channels, but the original channel count its returned into the channels_in_file property
		/*switch (channels_in_file)
		{
			case 1:
				format = GraphicsFormat::R8_UNORM;
				break;
			case 2:
				format = GraphicsFormat::RG8_UNORM;
				break;
			case 3:
				format = GraphicsFormat::RGB8_UNORM;
				break;
			case 4:
				format = GraphicsFormat::RGBA8_UNORM;
				break;
			default: break;
		}

		if (format == GraphicsFormat::Unknown)
		{
			stbi_image_free(data);
			AU_LOG_ERROR("Could not load image ", path.string(), " wrong format !");
			return nullptr;
		}*/

		// Resize if requested
		if (loadDesc.Width > 0 && loadDesc.Height > 0)
		{
			int targetWidth = std::min<int>(loadDesc.Width, width);
			int targetHeight = std::min<int>(loadDesc.Height, height);

			auto* resizedData = new uint8_t[targetWidth * targetHeight * channels_in_file];
			stbir_resize_uint8(data, width, height, 0, resizedData, targetWidth, targetHeight, 0, channels_in_file);
			stbi_image_free(data);

			width = targetWidth;
			height = targetHeight;
			data = resizedData;
		}

		String filename = path.filename().stem().string();
		std::transform(filename.begin(), filename.end(), filename.begin(), [](unsigned char c){ return std::tolower(c); });
		bool isNormalMap = filename.find("normal") != std::string::npos;

		ResourceName resourceName;
		resourceName.Name = path.string();

		if (loadDesc.GenerateMetaFile)
		{
			Path realPath;
			if (!fromAssetPackage && GetRealPath(path, realPath))
			{
				nlohmann::json metaFile = GetOrCreateMetaForPath(realPath, {
					{"srgb", !isNormalMap},
					{"normalMap", isNormalMap},
				});

				if (metaFile.contains("properties") && metaFile["properties"].contains("srgb"))
				{
					if (metaFile["properties"]["srgb"].get<bool>() && format == GraphicsFormat::RGBA8_UNORM)
					{
						format = GraphicsFormat::SRGBA8_UNORM;
					}
				}

				if(metaFile.contains("uuid"))
				{
					String uuid = metaFile["uuid"].get<String>();
					resourceName.ID = UUID::FromString<String>(uuid).value();
				}
				else
				{
					AU_LOG_WARNING("Texture ", path, " does not contain UUID");
				}
			}
		}

		if (loadDesc.ForceSRGB)
		{
			format = GraphicsFormat::SRGBA8_UNORM;
		}

		TextureDesc textureDesc;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = loadDesc.GenerateMips ? textureDesc.GetMipLevelCount() : 1;
		textureDesc.ImageFormat = format;
		textureDesc.Name = path.string();
		textureDesc.UseAsBindless = false;
		texture = m_RenderDevice->CreateTexture(textureDesc, nullptr);
		texture->SetResourceName(resourceName);

		for (unsigned int mipLevel = 0; mipLevel < textureDesc.MipLevels; mipLevel++)
		{
			m_RenderDevice->WriteTexture(texture, mipLevel, 0, data);

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

		if (!loadDesc.DoNotCache)
			m_LoadedTextures[path] = texture;

		return texture;
	}

	Texture_ptr ResourceManager::LoadTexture(const ResourceName& resourceName, const TextureLoadDesc& loadDesc)
	{
		// TODO: Finish resource name UUID's
		Path path = resourceName.Name;
		return LoadTexture(path, loadDesc);
	}

	Texture_ptr ResourceManager::LoadResourceIcon(const Path &path, int size)
	{
		TextureLoadDesc loadDesc = {
			.Width = size,
			.Height = size,
			.GenerateMips = false,
			.GenerateMetaFile = false,
			.ForceSRGB = false,
			.DoNotCache = true
		};

		return LoadTexture(path, loadDesc);
	}

	Texture_ptr ResourceManager::LoadLutTexture(const Path& path)
	{
		//Path hashPath = path / ".lut";

		if(m_LoadedTextures.find(path) != m_LoadedTextures.end()) {
			return m_LoadedTextures[path];
		}

		auto fileData = LoadFile(path);

		if(fileData.empty()) {
			return nullptr;
		}

		Texture_ptr texture = nullptr;

		int width,height,channels_in_file;
		uint8_t* data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(fileData.data()), fileData.size(), &width, &height, &channels_in_file, STBI_rgb_alpha);
		if(!data) {
			AU_LOG_ERROR("Cannot load Lut texture ! ", path.string());
			return nullptr;
		}

		int volumeWidth = width / height;
		int volumeHeight = height;
		int volumeDepth = volumeWidth;

		if(width % height > 0)
		{
			AU_LOG_ERROR("Texture needs to have depth slices multiply of height of the texture ! ", path.string());
			return nullptr;
		}

		TextureDesc textureDesc;
		textureDesc.Width = volumeWidth;
		textureDesc.Height = volumeHeight;
		textureDesc.DepthOrArraySize = volumeDepth;
		textureDesc.DimensionType = EDimensionType::TYPE_3D;
		textureDesc.MipLevels = 1;
		textureDesc.ImageFormat = GraphicsFormat::RGBA8_UNORM;
		textureDesc.Name = path.string();
		texture = m_RenderDevice->CreateTexture(textureDesc, nullptr);

		auto* slice = new uint8_t[volumeWidth * volumeHeight * 4];

		for (int d = 0; d < volumeDepth; ++d)
		{
			for (int y = 0; y < volumeHeight; ++y)
			{
				for (int x = 0; x < volumeWidth; ++x)
				{
					for (int c = 0; c < 4; ++c)
					{
						slice[(x + y * volumeWidth) * 4 + c] = data[((x + d * volumeDepth) + y * width) * 4 + c];
					}
				}
			}

			m_RenderDevice->WriteTexture(texture, 0, d, slice);
		}

		delete[] slice;

		m_LoadedTextures[path] = texture;
		return texture;
	}

	Mesh_ptr ResourceManager::LoadMesh(const Path& path)
	{
		auto fileData = LoadFile(path);

		if(fileData.empty()) {
			return nullptr;
		}

		Archive archive(fileData);
		int meshVersion;
		archive >> meshVersion;

		if (meshVersion != MESH_VERSION)
		{
			AU_LOG_ERROR("Incorrect amesh version !");
			return nullptr;
		}

		TTypeID meshType = Mesh::ReadMeshType(archive);

		if (meshType == StaticMesh::TypeID())
		{
			StaticMesh_ptr newMesh = std::make_shared<StaticMesh>();
			newMesh->Deserialize(archive);
			newMesh->UploadToGPU(false);
			return newMesh;
		}

		if (meshType == SkeletalMesh::TypeID())
		{
			// TODO: Implement skeletal mesh
		}

		return nullptr;
	}

	const MaterialDefinition_ptr& ResourceManager::GetOrLoadMaterialDefinition(const Path &path)
	{
		const auto& it = m_MaterialDefinitions.find(path);

		if(it != m_MaterialDefinitions.end())
		{
			return it->second;
		}

		nlohmann::json json;
		if(!LoadJson(path, json))
		{
			AU_LOG_FATAL("Cannot load engine without test material !");
		}

		MaterialDefinitionDesc materialDefinitionDesc;
		if(!MaterialLoader::ParseMaterialDefinitionJson(json, path, materialDefinitionDesc))
		{
			AU_LOG_FATAL("Cannot parse material json");
		}

		return (m_MaterialDefinitions[path] = std::make_shared<MaterialDefinition>(materialDefinitionDesc));
	}

	std::shared_ptr<Material> ResourceManager::LoadMaterial(const Path &path)
	{
		auto it = m_Materials.find(path);

		if (it != m_Materials.end())
		{
			return it->second;
		}

		nlohmann::json json;
		if(!LoadJson(path, json))
		{
			AU_LOG_FATAL("Cannot load engine without test material !");
		}

		if(!json.contains("base"))
		{
			AU_LOG_WARNING("Material ", path.string(), " does not have base file defined !");
			return nullptr;
		}

		Path definitionPath = json["base"].get<String>();

		const MaterialDefinition_ptr& materialDefinition = GetOrLoadMaterialDefinition(definitionPath);

		MaterialOverrides overrides;
		// TODO: Load overrides from file

		auto matInstance = materialDefinition->CreateInstance(overrides);

		m_Materials[path] = matInstance;

		return matInstance;
	}

	nlohmann::json ResourceManager::GetOrCreateMetaForPath(const Path& originalFilePath, const nlohmann::json &defaults)
	{
		Path metaPath = originalFilePath.string() + ".meta";

		nlohmann::json json;

		if (FileExists(metaPath))
		{
			LoadJson(metaPath, json);
			return json;
		}
		else
		{
			json["uuid"] = (String)UUID::Generate();
			json["properties"] = defaults;

			AU_LOG_INFO("Creating meta ", metaPath.string());

			std::ofstream stream;
			stream.open(metaPath);
			if(stream.is_open())
			{
				stream << std::setw(4) << json << std::endl;
				stream.close();
			}
		}

		return json;
	}

	void ResourceManager::ImportAsset(const Path& from, const Path& toFolder)
	{
		Path destPath = toFolder / from.filename();

		if (IsFileType(from, FT_IMAGE))
		{
			std::error_code errorCode;
			std::filesystem::copy(from, destPath, errorCode);

			if (errorCode)
			{
				AU_LOG_WARNING("Could not import asset: ", from.string(), ", reason: ", errorCode.message());
				return;
			}

			// TODO: Create meta file
			return;
		}

		if(IsFileType(from, FT_UNPROCESSED_MESH))
		{
			auto data = FS::LoadFile(from);

			MeshImportOptions meshImportOptions;
			meshImportOptions.UploadToGPU = false;
			meshImportOptions.KeepCPUData = true;
			meshImportOptions.PreTransform = true;
			meshImportOptions.SplitMeshes = false;

			AssimpModelLoader modelLoader;
			MeshImportedData importedData = modelLoader.ImportModel("Test", data, meshImportOptions);

			if(!importedData)
			{
				AU_LOG_ERROR("Could not import model: ", from);
				return;
			}

			bool hasMoreMeshes = !importedData.Meshes.empty();
			for (const auto& mesh : importedData.Meshes)
			{
				Archive archive;
				archive << MESH_VERSION;

				mesh->WriteMeshType(archive);
				mesh->Serialize(archive);

				if (!archive.GetSize())
				{
					AU_LOG_ERROR("Could not serialize mesh: ", from);
					return;
				}

				if (hasMoreMeshes)
					destPath = toFolder / (from.stem().string() + "_" + mesh->Name + ".amesh");
				else
					destPath = toFolder / (from.stem().string() + ".amesh");

				std::ofstream outStream;
				outStream.open(destPath, std::ios::out | std::ios::binary);
				outStream << archive;
				outStream.close();
			}

			return;
		}

		AU_LOG_WARNING("Import no implemented ! From: ", from.string(), ", To: ", toFolder.string());
	}

	void ResourceManager::UnloadAsset(const Path& path)
	{
		// TODO: Unload asset
	}

	bool ResourceManager::SaveCubeMapDef(const Path& path, const std::array<Texture_ptr, 6>& textures)
	{
		nlohmann::json json;
		nlohmann::json array = nlohmann::json::array();

		for (int i = 0; i < 6; ++i)
		{
			array[i] = textures[i]->GetResourceName();
		}

		json["textures"] = array;

		std::ofstream stream;
		stream.open(path);
		if(stream.is_open())
		{
			stream << std::setw(4) << json << std::endl;
			stream.close();
			return true;
		}

		return false;
	}

	Texture_ptr ResourceManager::LoadCubeMapDef(const Path& path)
	{
		nlohmann::json json;

		if(!LoadJson(path, json))
		{
			return nullptr;
		}

		std::array<Path, 6> textures;

		for (int i = 0; i < 6; ++i)
		{
			ResourceName resourceName(json["textures"][i].get<String>());

			// TODO: Implement resource name UUID
			textures[i] = resourceName.Name;
		}

		return GEngine->GetRenderManager()->CreateCubeMap(textures, false);
	}

	static const char* ImageExtensions[] = {
		".png",
		".jpg",
		".jpeg",
		".bmp",
		".tga"
	};

	static const char* ShaderExtensions[] = {
		".fss",
		".vss",
		".glsl",
		".vert",
		".frag",
		".geom",
		".compute"
	};

	static const char* UnprocessedMeshExtensions[] = {
		".fbx"
	};

	bool ResourceManager::IsFileType(const Path &path, FileType types)
	{
		Path extension = path.extension();

		if ((types & FT_IMAGE) == FT_IMAGE)
		{
			for (const auto& item : ImageExtensions)
			{
				if (item == extension)
				{
					return true;
				}
			}
		}

		if ((types & FT_MATERIAL_DEF) == FT_MATERIAL_DEF && extension == ".matd")
			return true;

		if ((types & FT_MATERIAL_INS) == FT_MATERIAL_INS && extension == ".mat")
			return true;

		if ((types & FT_SHADER) == FT_SHADER)
		{
			for (const auto &item : ShaderExtensions)
			{
				if (item == extension)
				{
					return true;
				}
			}
		}

		if ((types & FT_UNPROCESSED_MESH) == FT_UNPROCESSED_MESH)
		{
			for (const auto &item : UnprocessedMeshExtensions)
			{
				if (item == extension)
				{
					return true;
				}
			}
		}

		if ((types & FT_AMESH) == FT_AMESH  && extension == ".amesh")
			return true;

		if ((types & FT_CUBEMAP) == FT_CUBEMAP  && extension == ".cubemap")
			return true;

		return false;
	}

	static const char* IgnoredFileExtensions[] = {
		".meta"
	};

	bool ResourceManager::IsIgnoredFileType(const Path &path)
	{
		Path extension = path.extension();

		for (const auto& item : IgnoredFileExtensions)
		{
			if (item == extension)
			{
				return true;
			}
		}

		return false;
	}
}