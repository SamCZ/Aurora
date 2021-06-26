#pragma once

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Aurora/Framework/Mesh/StaticMesh.hpp>
#include <Aurora/Core/FileSystem.hpp>

namespace Aurora
{
	class ModelImporter
	{
	public:
		static StaticMesh_ptr LoadMesh(const DataBlob& dataBlob);
	};
}
