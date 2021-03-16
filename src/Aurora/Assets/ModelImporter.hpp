#pragma once

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <DataBlobImpl.hpp>
#include <RefCntAutoPtr.hpp>

#include <Aurora/Framework/Mesh/StaticMesh.hpp>

namespace Aurora
{
	class ModelImporter
	{
	public:
		static StaticMesh_ptr LoadMesh(const RefCntAutoPtr<IDataBlob>& dataBlob);
	};
}
