#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/Vector.hpp"
#include "Aurora/Core/Object.hpp"
#include "Aurora/Graphics/Base/Buffer.hpp"
#include "Aurora/Graphics/Material/Material.hpp"
#include "Aurora/Tools/robin_hood.h"
#include "VertexBuffer.hpp"

namespace Aurora
{
	typedef uint32_t Index_t;
	typedef uint32_t LOD;

	struct MaterialSlot
	{
		matref Material;
		String MaterialSlotName;
		std::map<String, Texture_ptr> Textures{};
		std::map<String, Vector4> Colors{};

		MaterialSlot() : Material(nullptr), MaterialSlotName(), Textures(), Colors() {}
		MaterialSlot(matref Material, String  materialSlotName) : Material(std::move(Material)), MaterialSlotName(std::move(materialSlotName)), Textures(), Colors() {}

		bool operator==(const MaterialSlot& left) const { return Material == left.Material && MaterialSlotName == left.MaterialSlotName; }
	};

	struct MeshSection
	{
		int32_t MaterialIndex;

		Index_t FirstIndex;
		Index_t NumTriangles;

		bool EnableCollision;
		bool CastShadow;

		EPrimitiveType PrimitiveType;

		MeshSection()
			: MaterialIndex(0)
			, FirstIndex(0)
			, NumTriangles(0)
			, EnableCollision(false)
			, CastShadow(true)
			, PrimitiveType(EPrimitiveType::TriangleList)
		{
		}
	};

	struct MeshLodResource
	{
		std::shared_ptr<IVertexBuffer> Vertices;
		std::vector<Index_t> Indices;
		std::vector<MeshSection> Sections;

		Buffer_ptr VertexBuffer;
		Buffer_ptr IndexBuffer;
		bool NeedUpdateBuffers;

		EIndexBufferFormat IndexFormat;

		MeshLodResource() : Vertices(nullptr), Indices(), Sections(), VertexBuffer(nullptr), IndexBuffer(nullptr), NeedUpdateBuffers(false), IndexFormat(EIndexBufferFormat::Uint32) { }
	};

	AU_CLASS(Mesh) : public ObjectBase
	{
	public:
		CLASS_OBJ(Mesh, ObjectBase);
		robin_hood::unordered_map<LOD, MeshLodResource> LODResources;
		std::vector<MaterialSlot> MaterialSlots;

		void UploadToGPU(bool keepCPUData, bool dynamic = false);
	};

	template<typename Self>
	class MeshBufferHelper
	{
	public:
		bool HasVertexBuffer(int lod = 0)
		{
			if(!static_cast<Self*>(this)->LODResources.contains(lod)) {
				return false;
			}

			MeshLodResource& lodResource = static_cast<Self*>(this)->LODResources[lod];

			if(lodResource.Vertices == nullptr) {
				return false;
			}

			return true;
		}

		template<typename BufferTypename>
		VertexBuffer<BufferTypename>* GetVertexBuffer(LOD lod = 0)
		{
			if(!static_cast<Self*>(this)->LODResources.contains(lod)) {
				return nullptr;
			}

			MeshLodResource& lodResource = static_cast<Self*>(this)->LODResources[lod];

			if(lodResource.Vertices == nullptr) {
				return nullptr;
			}
#ifdef DEBUG
			return dynamic_cast<VertexBuffer<BufferTypename>*>(lodResource.Vertices);
#else
			return static_cast<VertexBuffer<BufferTypename>*>(lodResource.Vertices.get());
#endif
		}

		std::vector<Index_t> GetIndexBuffer(int lod = 0)
		{
			if(!static_cast<Self*>(this)->LODResources.contains(lod)) {
				return {};
			}

			return static_cast<Self*>(this)->LODResources[lod].Indices;
		}

		template<typename BufferTypename>
		VertexBuffer<BufferTypename>* CreateVertexBuffer(int lod = 0, MeshLodResource** out_resource = nullptr)
		{
			MeshLodResource* lodResource;

			if(static_cast<Self*>(this)->LODResources.contains(lod)) {
				lodResource = &static_cast<Self*>(this)->LODResources[lod];
			} else {
				static_cast<Self*>(this)->LODResources[lod] = {};
				lodResource = &static_cast<Self*>(this)->LODResources[lod];
			}

			if(lodResource->Vertices == nullptr) {
				lodResource->Vertices = std::make_shared<VertexBuffer<BufferTypename>>();
			}

			if(out_resource != nullptr) {
				*out_resource = lodResource;
			}

			return (VertexBuffer<BufferTypename>*)lodResource->Vertices.get();
		}
	};

	AU_CLASS(StaticMesh) : public Mesh, public MeshBufferHelper<StaticMesh>
	{
	public:
		CLASS_OBJ(StaticMesh, Mesh);

		struct Vertex
		{
			Vector3 Position;
			Vector2 TexCoord;
			Vector3 Normal;
			Vector3 Tangent;
			Vector3 BiTangent;
		};
	};

	AU_CLASS(SkeletalMesh) : public Mesh, public MeshBufferHelper<SkeletalMesh>
	{
	public:
		CLASS_OBJ(SkeletalMesh, Mesh);

		struct Vertex
		{
			Vector3 Position;
			Vector2 TexCoord;
			Vector3 Normal;
			Vector3 Tangent;
			Vector3 BiTangent;

			Vector4ui BoneIndices;
			Vector4 BoneWeights;
		};
	};
}