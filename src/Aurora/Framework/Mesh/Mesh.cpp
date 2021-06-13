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
				BufferDesc desc;
				desc.Name = "Mesh Buffer";
				desc.Usage = EBufferUsage::StaticDraw;
				desc.Stride = resource.Vertices->GetStride();
				desc.ByteSize = resource.Vertices->GetSize();
				desc.Type = EBufferType::VertexBuffer;
				resource.VertexBuffer = RD->CreateBuffer(desc);
			} else {
				// TODO: Updated vertex buffer
			}

			if(resource.Indices.empty()) {
				continue;
			}

			if(resource.IndexBuffer == nullptr) {
				BufferDesc desc;
				desc.Name = "Mesh Index Buffer";
				desc.Usage = EBufferUsage::StaticDraw;
				desc.Stride = sizeof(Index_t);
				desc.ByteSize = resource.Indices.size() * sizeof(Index_t);
				desc.Type = EBufferType::IndexBuffer;
				resource.VertexBuffer = RD->CreateBuffer(desc);
			} else {
				// TODO: Updated index buffer
			}
		}
	}
}