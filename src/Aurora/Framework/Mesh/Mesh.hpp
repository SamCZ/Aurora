#pragma once

#include "Aurora/Core/Types.hpp"
#include "Aurora/Core/Vector.hpp"
#include "Aurora/Graphics/Base/Buffer.hpp"
#include "Aurora/Graphics/Material/SMaterial.hpp"
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

		MeshSection()
			: MaterialIndex(0)
			, FirstIndex(0)
			, NumTriangles(0)
			, EnableCollision(false)
			, CastShadow(true)
		{
		}
	};

	struct MeshLodResource
	{
		IVertexBuffer* Vertices;
		std::vector<Index_t> Indices;
		std::vector<MeshSection> Sections;

		Buffer_ptr VertexBuffer;
		Buffer_ptr IndexBuffer;
		bool NeedUpdateBuffers;

		MeshLodResource() : Vertices(nullptr), Indices(), Sections(), VertexBuffer(nullptr), IndexBuffer(nullptr), NeedUpdateBuffers(false) { }
	};

	AU_CLASS(Mesh)
	{
	public:
		robin_hood::unordered_map<uint8_t, MeshLodResource> LODResources;
		std::vector<MaterialSlot> MaterialSlots;
	};

	AU_CLASS(StaticMesh) : public Mesh
	{
	public:
		struct Vertex
		{
			Vector3 Position;
			Vector2 TexCoord;
			Vector3 Normal;
			Vector3 Tangent;
			Vector3 BiTangent;
		};
	};

	AU_CLASS(SkeletalMesh) : public Mesh
	{
	public:
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