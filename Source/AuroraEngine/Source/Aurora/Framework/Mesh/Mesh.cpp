#include "Mesh.hpp"

using namespace Diligent;

namespace Aurora::Framework
{

    // TODO: Move this method to render interface
    void HMesh::UpdateBuffers(Diligent::RefCntAutoPtr<Diligent::IRenderDevice>& renderDevice, Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext)
    {
        for(auto& it : LODResources) {
            HMeshLodResource& resource = it.second;

            if(resource.Vertices == nullptr || resource.Vertices->GetCount() == 0) {
                continue;
            }

            if(resource.VertexBuffer == nullptr) {
                BufferDesc VertBuffDesc;
                VertBuffDesc.Name          = "Mesh vertex buffer";
                VertBuffDesc.Usage         = USAGE_IMMUTABLE;
                VertBuffDesc.BindFlags     = BIND_VERTEX_BUFFER;
                VertBuffDesc.uiSizeInBytes = resource.Vertices->GetStride();
                BufferData VBData;
                VBData.pData    = resource.Vertices->GetData();
                VBData.DataSize = resource.Vertices->GetSize();
                renderDevice->CreateBuffer(VertBuffDesc, &VBData, &resource.VertexBuffer);
            } else {
                immediateContext->UpdateBuffer(resource.VertexBuffer, 0, resource.Vertices->GetSize(), resource.Vertices->GetData(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }

            if(resource.Indices.empty()) {
                continue;
            }

            if(resource.IndexBuffer == nullptr) {
                BufferDesc IndBuffDesc;
                IndBuffDesc.Name          = "Mesh index buffer";
                IndBuffDesc.Usage         = USAGE_IMMUTABLE;
                IndBuffDesc.BindFlags     = BIND_INDEX_BUFFER;
                IndBuffDesc.uiSizeInBytes = sizeof(uint32_t);
                BufferData IBData;
                IBData.pData    = resource.Indices.data();
                IBData.DataSize = resource.Indices.size() * sizeof(uint32_t);
                renderDevice->CreateBuffer(IndBuffDesc, &IBData, &resource.IndexBuffer);
            } else {
                immediateContext->UpdateBuffer(resource.IndexBuffer, 0, resource.Indices.size() * sizeof(uint32_t), resource.Indices.data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }
        }
    }
}