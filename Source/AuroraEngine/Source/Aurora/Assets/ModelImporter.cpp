#include "ModelImporter.hpp"

using namespace Aurora::Render;

namespace Aurora
{
    static std::vector<std::string> TextureTypeEnumToString = {
            "aiTextureType_NONE", "aiTextureType_DIFFUSE", "aiTextureType_SPECULAR",
            "aiTextureType_AMBIENT", "aiTextureType_EMISSIVE", "aiTextureType_HEIGHT",
            "aiTextureType_NORMALS", "aiTextureType_SHININESS", "aiTextureType_OPACITY",
            "aiTextureType_DISPLACEMENT", "aiTextureType_LIGHTMAP", "aiTextureType_REFLECTION",
            "aiTextureType_UNKNOWN", "_aiTextureType_Force32Bit" };

    static Map<uint8_t, String> PrimitiveTypeToString {
            {(uint8_t)aiPrimitiveType::aiPrimitiveType_POINT, "aiPrimitiveType_POINT"},
            {(uint8_t)aiPrimitiveType::aiPrimitiveType_LINE, "aiPrimitiveType_LINE"},
            {(uint8_t)aiPrimitiveType::aiPrimitiveType_TRIANGLE, "aiPrimitiveType_TRIANGLE"},
            {(uint8_t)aiPrimitiveType::aiPrimitiveType_POLYGON, "aiPrimitiveType_POLYGON"},
            {(uint8_t)aiPrimitiveType::_aiPrimitiveType_Force32Bit, "_aiPrimitiveType_Force32Bit"}
    };

    static inline Vector3 vec3_cast(const aiVector3D &v) { return glm::vec3(v.x, v.y, v.z); }
    static inline Vector2 vec2_cast(const aiVector3D &v) { return glm::vec2(v.x, v.y); }
    static inline Quaternion quat_cast(const aiQuaternion &q) { return glm::quat(q.w, q.x, q.y, q.z); }
    static inline Matrix4 mat4_cast(const aiMatrix4x4 &m) { return glm::transpose(glm::make_mat4(&m.a1)); }
    static inline Matrix4 mat4_cast(const aiMatrix3x3 &m) { return glm::transpose(glm::make_mat3(&m.a1)); }

    static constexpr uint32_t MESH_PROCESS_FLAGS = aiProcessPreset_TargetRealtime_Quality |
                                           aiProcess_ValidateDataStructure |
                                           aiProcess_OptimizeMeshes |
                                           aiProcess_FlipUVs;

    template<typename MeshProcessor, typename MeshType>
    static void ProcessNodes(MeshProcessor meshProcessor, MeshType* engineMesh, const aiScene* scene, aiNode* node, int& materialSlotIndex);

    static void ProcessSkeletalMesh(aiMesh* mesh, aiNode* node, HStaticMesh* staticMesh, int materialSlotIndex);

    HStaticMeshPtr FModelImporter::LoadMesh(const RefCntAutoPtr<IDataBlob>& dataBlob)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(dataBlob->GetConstDataPtr(), dataBlob->GetSize(), MESH_PROCESS_FLAGS | aiProcess_PreTransformVertices);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LogError("ModelImporter::Import", importer.GetErrorString());
            return nullptr;
        }

        HStaticMeshPtr mesh = New(HStaticMesh);

        mesh->LODResources[0] = HMeshLodResource();
        mesh->LODResources[0].Vertices = new VertexBuffer<StaticMeshVertexBufferEntry>();

        int materialSlotIndex = 0;
        ProcessNodes(&ProcessSkeletalMesh, mesh.get(), scene, scene->mRootNode, materialSlotIndex);

        return mesh;
    }

    template<typename MeshProcessor, typename MeshType>
    static void ProcessNodes(MeshProcessor meshProcessor, MeshType* engineMesh, const aiScene* scene, aiNode* node, int& materialSlotIndex)
    {
        List<aiMesh*> triangleMeshes;

        // Setup triangle meshes
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            if (mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
            {
                triangleMeshes.push_back(mesh);
            }
        }

        for (auto mesh : triangleMeshes) {
            int meshMaterialSlot = 0;

            if (mesh->mMaterialIndex >= 0) {
                meshMaterialSlot = materialSlotIndex++;

                FMaterialSlot materialSlot = {};
                materialSlot.Material = nullptr;
                materialSlot.MaterialSlotName = String("Material_") + ToString(meshMaterialSlot);
                engineMesh->MaterialSlots[meshMaterialSlot] = materialSlot;

                aiMaterial* sourceMaterial = scene->mMaterials[mesh->mMaterialIndex];

                for (uint8_t texType = 0; texType < (uint8_t)aiTextureType::aiTextureType_UNKNOWN; texType++) {
                    unsigned int textureCount = sourceMaterial->GetTextureCount((aiTextureType)texType);

                    if (textureCount == 0)
                    {
                        continue;
                    }

                    aiString texturePathAiStr;

                    if (sourceMaterial->GetTexture((aiTextureType)texType, 0, &texturePathAiStr) != aiReturn_SUCCESS)
                    {
                        continue;
                    }

                    Path filePath = texturePathAiStr.C_Str();

                    Log(TextureTypeEnumToString[texType], filePath.string());
                }
            }

            meshProcessor(mesh, node, engineMesh, meshMaterialSlot);
        }

        // Iterate over children
        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            ProcessNodes(meshProcessor, engineMesh, scene, node->mChildren[i], materialSlotIndex);
        }
    }

    static void ProcessSkeletalMesh(aiMesh* mesh, aiNode* node, HStaticMesh* staticMesh, int materialSlotIndex)
    {
        auto& indices = staticMesh->LODResources[0].Indices;
        auto* buffer = dynamic_cast<VertexBuffer<StaticMeshVertexBufferEntry>*>(staticMesh->LODResources[0].Vertices);

        FMeshSection section;
        section.MaterialIndex = materialSlotIndex;

        int lastIndex = buffer->GetCount();

        for (uint32_t vertIdx = 0u; vertIdx < mesh->mNumVertices; vertIdx++)
        {
            aiVector3D vert = mesh->mVertices[vertIdx];
            aiVector3D norm = mesh->mNormals[vertIdx];

            StaticMeshVertexBufferEntry vertexEntry = {};

            vertexEntry.Position = vec3_cast(vert);
            vertexEntry.Normal = vec3_cast(norm);

            if(mesh->HasTextureCoords(0)) {
                aiVector3D tex = mesh->mTextureCoords[0][vertIdx];
                vertexEntry.TexCoord = vec2_cast(tex);
            }

            if(mesh->HasTangentsAndBitangents()) {
                aiVector3D tan = mesh->mTangents[vertIdx];
                aiVector3D bit = mesh->mBitangents[vertIdx];
                vertexEntry.Tangent = vec3_cast(tan);
                vertexEntry.BiTangent = vec3_cast(bit);
            }

            buffer->Add(vertexEntry);
        }

        section.FirstIndex = indices.size();

        for (std::uint32_t faceIdx = 0u; faceIdx < mesh->mNumFaces; faceIdx++) {
            for (uint8_t id = 0; id < 3; id++)
            {
                uint32_t index = mesh->mFaces[faceIdx].mIndices[id];

                uint32_t newIndex = lastIndex + index;

                indices.push_back(newIndex);
            }
        }

        section.NumTriangles = mesh->mNumFaces * 3;

        staticMesh->LODResources[0].Sections.push_back(section);
    }
}