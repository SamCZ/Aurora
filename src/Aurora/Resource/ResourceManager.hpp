#pragma once

#include <unordered_map>
#include "Aurora/Core/Common.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/Material/MaterialDefinition.hpp"
#include "AssetBank.hpp"

#include <nlohmann/json.hpp>

namespace Aurora
{
	enum FileType : uint32_t
	{
		FT_IMAGE = BITF(0),
		FT_MATERIAL_DEF = BITF(1),
		FT_MATERIAL_INS = BITF(2),

		FT_SHADER = BITF(3),
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

	class AU_API ResourceManager
	{
	private:
		IRenderDevice* m_RenderDevice;
		std::vector<Path> m_FileSearchPaths;
		std::map<Path, std::pair<Path, ABankHeader>> m_AssetPackageFiles;
		std::map<Path, std::vector<Path>> m_AssetPackageFolders;
		std::map<String, Shader_ptr> m_ShaderPrograms;
		std::map<Path, Texture_ptr> m_LoadedTextures;
		std::map<Path, MaterialDefinition_ptr> m_MaterialDefinitions;
	public:
		explicit ResourceManager(IRenderDevice* renderDevice);

		void AddFileSearchPath(const String& path);

		void LoadPackageFile(const Path& path);

		DataBlob LoadFile(const Path& path, bool* isFromAssetPackage = nullptr);
		[[nodiscard]] bool FileExists(const Path& path, bool* isFromAssetPackage = nullptr) const;
		[[nodiscard]] bool GetRealPath(const Path& path, Path& path_out) const;
		String LoadFileToString(const Path& path, bool* isFromAssetPackage = nullptr);

		String ReadShaderSource(const Path& path, std::vector<Path>& alreadyIncluded);
		String ReadShaderSource(const Path& path)
		{
			std::vector<Path> empty;
			return ReadShaderSource(path, empty);
		}

		ShaderProgramDesc CreateShaderProgramDesc(const String& name, const std::map<EShaderType, Path>& shaderTypesPaths, const ShaderMacros &macros = {});
		Shader_ptr LoadShader(const String& name, const std::map<EShaderType, Path>& shaderTypesPaths, const ShaderMacros& macros = {});
		inline Shader_ptr LoadShader(const String& name, const Path& vertexShader, const Path& fragmentShader, const ShaderMacros& macros = {})
		{
			return LoadShader(name, { {EShaderType::Vertex, vertexShader}, {EShaderType::Pixel, fragmentShader} }, macros);
		}
		Shader_ptr LoadComputeShader(const Path& path, const ShaderMacros& macros = {});

		bool LoadJson(const Path &path, nlohmann::json &json);

		Texture_ptr LoadTexture(const Path& path, const TextureLoadDesc& loadDesc = TextureLoadDesc());
		Texture_ptr LoadIcon(const Path& path, int size = 0);
		Texture_ptr LoadLutTexture(const Path& path);

		const MaterialDefinition_ptr& GetOrLoadMaterialDefinition(const Path& path);
		std::shared_ptr<Material> LoadMaterial(const Path& path);

		nlohmann::json GetOrCreateMetaForPath(const Path& path, const nlohmann::json& defaults);

		void ImportAsset(const Path& from, const Path& to);
		void UnloadAsset(const Path& path);

		[[nodiscard]] inline const std::vector<Path>& GetFileSearchPaths() const { return m_FileSearchPaths; }

		static bool IsFileType(const Path& path, FileType types);
		static bool IsIgnoredFileType(const Path& path);
	};
}
