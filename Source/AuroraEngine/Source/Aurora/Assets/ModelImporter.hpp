#pragma once

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <DataBlobImpl.hpp>
#include <RefCntAutoPtr.hpp>

#include "../Framework/Mesh/Mesh.hpp"
#include "../Framework/Animation/Animation.hpp"
#include "../Framework/Animation/Armature.hpp"

#include "../Framework/Mesh/StaticMesh.hpp"
#include "../Framework/Mesh/SkeletalMesh.hpp"

using namespace Diligent;
using namespace Aurora::Framework;

namespace Aurora
{
    class FModelImporter
    {
    public:
        static HStaticMeshPtr LoadMesh(const RefCntAutoPtr<IDataBlob>& dataBlob);
        //static void LoadAnimatedMesh(const RefCntAutoPtr<IDataBlob>& dataBlob);
        //static FAnimation LoadAnimation(const FArmature& armature, const RefCntAutoPtr<IDataBlob>& dataBlob);
    };
}