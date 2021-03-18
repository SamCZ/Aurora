#pragma once

#include <Aurora/Core/Common.hpp>
#include <Aurora/Graphics/VertexBuffer.hpp>
#include <Aurora/Graphics/Material.hpp>
#include <Aurora/Physics/Bound.hpp>

#include <Buffer.h>
#include <RenderDevice.h>
#include <RefCntAutoPtr.hpp>
#include <utility>

namespace Aurora
{
	struct MaterialSlot
	{
		Material_ptr Material;
		String MaterialSlotName;

		MaterialSlot() : Material(nullptr), MaterialSlotName() {}
		MaterialSlot(Material_ptr Material, String  materialSlotName) : Material(std::move(Material)), MaterialSlotName(std::move(materialSlotName)) {}

		bool operator==(const MaterialSlot& left) const { return Material == left.Material && MaterialSlotName == left.MaterialSlotName; }
	};

	struct MeshSection
	{
		int32_t MaterialIndex;

		uint32_t FirstIndex;
		uint32_t NumTriangles;

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
		std::vector<uint32_t> Indices;
		std::vector<MeshSection> Sections;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> IndexBuffer;
		bool NeedUpdateBuffers;
	};

	AU_CLASS(Mesh)
	{
	public:
		typedef std::vector<uint32_t> IndexBufferType;
	public:
		virtual ~Mesh();
	public:
		std::unordered_map<uint8_t, MeshLodResource> LODResources;
		std::unordered_map<uint8_t, MaterialSlot> MaterialSlots;
		std::unique_ptr<Bound> m_Bounds;
	public:
		virtual LayoutElement* GetLayout() const = 0;
		virtual int GetLayoutElementCount() const = 0;

		void UpdateBuffers(Diligent::RefCntAutoPtr<Diligent::IRenderDevice>& renderDevice, Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext);

	public:
		template<class T>
		inline void SetBounds(const T& bound)
		{
			m_Bounds = std::make_unique<T>(bound);
		}

		const std::unique_ptr<Bound>& GetBounds() const
		{
			return m_Bounds;
		}
	};

	template<typename MeshType, typename BufferTypename>
	class MeshBufferHelper
	{
		friend class Mesh;
	private:
		MeshType* m_Mesh;
	public:
		typedef VertexBuffer<BufferTypename> BufferType;

		inline explicit MeshBufferHelper(MeshType* selfInstance) : m_Mesh(selfInstance)
		{

		}

		virtual ~MeshBufferHelper() = default;

		inline bool HasVertexBuffer(int lod = 0)
		{
			if(!m_Mesh->LODResources.contains(lod)) {
				return false;
			}

			MeshLodResource& lodResource = m_Mesh->LODResources[lod];

			if(lodResource.Vertices == nullptr) {
				return false;
			}

			return true;
		}

		inline VertexBuffer<BufferTypename>* GetVertexBuffer(int lod = 0)
		{
			if(!m_Mesh->LODResources.contains(lod)) {
				return nullptr;
			}

			MeshLodResource& lodResource = m_Mesh->LODResources[lod];

			if(lodResource.Vertices == nullptr) {
				return nullptr;
			}
#ifdef DEBUG
			return dynamic_cast<VertexBuffer<BufferTypename>*>(lodResource.Vertices);
#else
			return static_cast<VertexBuffer<BufferTypename>*>(lodResource.Vertices);
#endif
		}

		inline VertexBuffer<BufferTypename>* CreateVertexBuffer(int lod = 0, MeshLodResource** out_resource = nullptr)
		{
			MeshLodResource* lodResource;

			if(m_Mesh->LODResources.contains(lod)) {
				lodResource = &m_Mesh->LODResources[lod];
			} else {
				m_Mesh->LODResources[lod] = {};
				lodResource = &m_Mesh->LODResources[lod];
			}

			if(lodResource->Vertices == nullptr) {
				lodResource->Vertices = new VertexBuffer<BufferTypename>();
			}

			if(out_resource != nullptr) {
				*out_resource = lodResource;
			}

			return (VertexBuffer<BufferTypename>*)lodResource->Vertices;
		}
	};
}