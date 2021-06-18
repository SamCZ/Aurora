#include "Mesh.hpp"
#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{
	Mesh::~Mesh()
	{
		for(auto& resource : LODResources) {
			delete resource.second.Vertices;
		}
	}

	// TODO: Move this method to render interface
	void Mesh::UpdateBuffers()
	{
		for(auto& it : LODResources) {
			MeshLodResource& resource = it.second;

			if(resource.Vertices == nullptr || resource.Vertices->GetCount() == 0) {
				continue;
			}

			if(resource.VertexBuffer == nullptr) {
				assert(resource.Vertices->GetSize() > 0);

				BufferDesc desc;
				desc.Name = "Mesh Buffer";
				desc.Usage = EBufferUsage::StaticDraw;
				desc.Stride = resource.Vertices->GetStride();
				desc.ByteSize = resource.Vertices->GetSize();
				desc.Type = EBufferType::VertexBuffer;
				resource.VertexBuffer = RD->CreateBuffer(desc, resource.Vertices->GetData());
			} else {
				// TODO: Updated vertex buffer
			}

			if(resource.Indices.empty()) {
				continue;
			}

			if(resource.IndexBuffer == nullptr) {
				assert(resource.Indices.size() > 0);
				BufferDesc desc;
				desc.Name = "Mesh Index Buffer";
				desc.Usage = EBufferUsage::StaticDraw;
				desc.Stride = sizeof(Index_t);
				desc.ByteSize = resource.Indices.size() * sizeof(Index_t);
				desc.Type = EBufferType::IndexBuffer;
				resource.IndexBuffer = RD->CreateBuffer(desc, resource.Indices.data());
			} else {
				// TODO: Updated index buffer
			}
		}
	}
}