#include "Mesh.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"

namespace Aurora
{
	void Mesh::UploadToGPU(bool keepCPUData, bool dynamic)
	{
		for(auto& it : LODResources)
		{
			MeshLodResource& lodResource = it.second;

			if(!lodResource.Vertices || lodResource.Vertices->GetCount() == 0)
			{
				AU_LOG_WARNING("Mesh LOD", (int)it.first, " has empty vertices !");
				continue;
			}

			if(lodResource.VertexBuffer && lodResource.VertexBuffer->GetDesc().ByteSize == lodResource.Vertices->GetSize())
			{

			}
			else
			{
				lodResource.VertexBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("Mesh VB", lodResource.Vertices->GetSize(), EBufferType::VertexBuffer, dynamic ? EBufferUsage::DynamicDraw : EBufferUsage::StaticDraw));
			}

			GEngine->GetRenderDevice()->WriteBuffer(lodResource.VertexBuffer, lodResource.Vertices->GetData());

			if(!keepCPUData)
				lodResource.Vertices.reset();

			if(lodResource.Indices.empty())
			{
				continue;
			}

			if(lodResource.IndexBuffer && lodResource.IndexBuffer->GetDesc().ByteSize == lodResource.Indices.size() * sizeof(Index_t))
			{

			}
			else
			{
				lodResource.IndexBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("Mesh IB", lodResource.Indices.size() * sizeof(Index_t), EBufferType::IndexBuffer, dynamic ? EBufferUsage::DynamicDraw : EBufferUsage::StaticDraw));
			}

			GEngine->GetRenderDevice()->WriteBuffer(lodResource.IndexBuffer, lodResource.Indices.data());

			if(!keepCPUData)
				lodResource.Indices.clear();
		}
	}
}