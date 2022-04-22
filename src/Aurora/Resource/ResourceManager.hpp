#pragma once

#include <unordered_map>
#include "Aurora/Core/Common.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/FileWatcher.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/Material/MaterialDefinition.hpp"
#include "Aurora/Framework/Mesh/Mesh.hpp"
#include "AssetBank.hpp"
#include "FileTree.hpp"
#include "ResourceName.hpp"

#include <nlohmann/json.hpp>

namespace Aurora
{
	struct path_hash
	{
		std::size_t operator()(const Path& path) const {
			return hash_value(path);
		}
	};

	enum FileType : uint32_t
	{
		FT_IMAGE = BITF(0),
		FT_MATERIAL_DEF = BITF(1),
		FT_MATERIAL_INS = BITF(2),

		FT_SHADER = BITF(3),

		FT_UNPROCESSED_MESH = BITF(4),
		FT_AMESH = BITF(5),
		FT_CUBEMAP = BITF(6)
	};

	struct TextureLoadDesc
	{
		int Width = 0;
		int Height = 0;
		bool GenerateMips = true;
		bool GenerateMetaFile = true;
		bool ForceSRGB = false;
		bool DoNotCache = false;
	};

	struct FileTreeContainer
	{
		Path Root;
		FileTree* Tree;
		Aurora::FileWatcher* FileWatcher;

		~FileTreeContainer()
		{
			delete FileWatcher;
			delete Tree;
		}

		void OnTreeChanged(EFileAction action, const Path& path, const Path& prevPath);
	};

	class AU_API ResourceManager
	{
	private:
		static const int MESH_VERSION = 1;

		IRenderDevice* m_RenderDevice;
		std::vector<Path> m_FileSearchPaths;
		std::unordered_map<Path, FileTreeContainer*, path_hash> m_FileTrees;
		std::unordered_map<Path, std::pair<Path, ABankHeader>, path_hash> m_AssetPackageFiles;
		std::unordered_map<Path, std::vector<Path>, path_hash> m_AssetPackageFolders;
		std::unordered_map<Path, Shader_ptr, path_hash> m_ShaderPrograms;
		std::unordered_map<Path, Texture_ptr, path_hash> m_LoadedTextures;
		std::unordered_map<Path, MaterialDefinition_ptr, path_hash> m_MaterialDefinitions;
		std::unordered_map<Path, Material_ptr, path_hash> m_Materials;
	public:
		explicit ResourceManager(IRenderDevice* renderDevice);
		~ResourceManager();

		void Update();

		void AddFileSearchPath(const String& path);
		FileTree* CreateFileTree(const Path& path, bool watch);
		FileTree* GetFileTree(const Path& path);

		void LoadPackageFile(const Path& path);

		DataBlob LoadFile(const Path& path, bool* isFromAssetPackage = nullptr) const;
		[[nodiscard]] bool FileExists(const Path& path, bool* isFromAssetPackage = nullptr) const;
		[[nodiscard]] bool GetRealPath(const Path& path, Path& path_out) const;
		String LoadFileToString(const Path& path, bool* isFromAssetPackage = nullptr) const;

		String ReadShaderSource(const Path& path, std::vector<Path>& alreadyIncluded) const;
		String ReadShaderSource(const Path& path) const
		{
			std::vector<Path> empty;
			return ReadShaderSource(path, empty);
		}

		bool LoadShaderProgramSources(ShaderProgramDesc& shaderProgramDesc);
		ShaderProgramDesc CreateShaderProgramDesc(const String& name, const std::map<EShaderType, Path>& shaderTypesPaths, const ShaderMacros &macros = {});
		Shader_ptr LoadShader(const String& name, const std::map<EShaderType, Path>& shaderTypesPaths, const ShaderMacros& macros = {});
		inline Shader_ptr LoadShader(const String& name, const Path& vertexShader, const Path& fragmentShader, const ShaderMacros& macros = {})
		{
			return LoadShader(name, { {EShaderType::Vertex, vertexShader}, {EShaderType::Pixel, fragmentShader} }, macros);
		}
		Shader_ptr LoadComputeShader(const Path& path, const ShaderMacros& macros = {});

		bool LoadJson(const Path &path, nlohmann::json &json);

		Texture_ptr LoadTexture(const Path& path, const TextureLoadDesc& loadDesc = TextureLoadDesc());
		Texture_ptr LoadTexture(const ResourceName& resourceName, const TextureLoadDesc& loadDesc = TextureLoadDesc());
		Texture_ptr LoadResourceIcon(const Path& path, int size = 0);
		Texture_ptr LoadLutTexture(const Path& path);

		Mesh_ptr LoadMesh(const Path& path);

		const MaterialDefinition_ptr& GetOrLoadMaterialDefinition(const Path& path);
		std::shared_ptr<Material> LoadMaterial(const Path& path);

		nlohmann::json GetOrCreateMetaForPath(const Path& path, const nlohmann::json& defaults);

		void ImportAsset(const Path& from, const Path& to);
		void UnloadAsset(const Path& path);

		bool SaveCubeMapDef(const Path& path, const std::array<Texture_ptr, 6>& textures);
		Texture_ptr LoadCubeMapDef(const Path& path);

		[[nodiscard]] inline const std::vector<Path>& GetFileSearchPaths() const { return m_FileSearchPaths; }
		[[nodiscard]] inline const std::unordered_map<Path, MaterialDefinition_ptr, path_hash>& GetMaterialDefs() const { return m_MaterialDefinitions; }

		static bool IsFileType(const Path& path, FileType types);
		static bool IsIgnoredFileType(const Path& path);
	};
}
