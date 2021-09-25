#pragma once

#include "Aurora/Core/Common.hpp"
#include "Aurora/Core/String.hpp"
#include "Aurora/Core/Types.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "AssetBank.hpp"

namespace Aurora
{
	class ResourceManager
	{
	private:
		IRenderDevice* m_RenderDevice;
		std::vector<Path> m_FileSearchPaths;
		std::map<Path, std::pair<Path, ABankHeader>> m_AssetPackageFiles;
		std::map<Path, std::vector<Path>> m_AssetPackageFolders;
		std::map<Path, Shader_ptr> m_ShaderPrograms;
	public:
		explicit ResourceManager(IRenderDevice* renderDevice);

		void AddFileSearchPath(const String& path);

		void LoadPackageFile(const Path& path);

		DataBlob LoadFile(const Path& path, bool* isFromAssetPackage = nullptr);
		[[nodiscard]] bool FileExists(const Path& path, bool* isFromAssetPackage = nullptr) const;
		String LoadFileToString(const Path& path, bool* isFromAssetPackage = nullptr);

		String ReadShaderSource(const Path& path, std::vector<Path>& alreadyIncluded);
		String ReadShaderSource(const Path& path)
		{
			std::vector<Path> empty;
			return ReadShaderSource(path, empty);
		}

		Shader_ptr LoadShader(const String& name, const std::map<EShaderType, Path>& shaderTypesPaths, const ShaderMacros& macros = {});
		Shader_ptr LoadComputeShader(const Path& path, const ShaderMacros& macros = {});
	};
}
