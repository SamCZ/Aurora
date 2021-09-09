#pragma once

#include <Aurora/Core/FileSystem.hpp>
#include <Aurora/Framework/Mesh/Mesh.hpp>

namespace Aurora
{
	class AssetManager;

	class GLTFLoader
	{
	private:
		AssetManager* m_AssetManager;
	public:
		inline explicit GLTFLoader(AssetManager* assetManager) : m_AssetManager(assetManager) { }

		Mesh_ptr Load(const Path& path);
	};
}