#pragma once

#include "Aurora/Core/Common.hpp"

namespace Aurora
{
	class Material;

	class MaterialLoader final
	{
	private:
		enum class LoadType : uint8_t
		{
			MaterialFile = 0,
			Compute,
			ShaderFolder
		};
	private:
		LoadType m_LoadType;
		Path m_Path;
		std::map<String, String> m_Macros;
	protected:
		MaterialLoader(const LoadType& loadType, Path  path);
	public:
		static MaterialLoader LoadShaderFolder(const Path& path);
		static MaterialLoader LoadCompute(const Path& path);
		static MaterialLoader Load(const Path& path);

		MaterialLoader& AddMacro(const String& macro, const String& value);

		std::shared_ptr<Material> Finish();

	private:
		std::shared_ptr<Material> LoadMaterialFile(const Path& path, std::map<String, String> macros);
	};
}