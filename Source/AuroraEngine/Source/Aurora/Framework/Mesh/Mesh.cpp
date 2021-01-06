#include "Mesh.hpp"

using namespace Diligent;

namespace Aurora::Framework
{
    HMesh::~HMesh()
    {
        for(auto& resource : LODResources) {
            delete resource.second.Vertices;
        }
    }

    // TODO: Move this method to render interface
    void HMesh::UpdateBuffers(Diligent::RefCntAutoPtr<Diligent::IRenderDevice>& renderDevice, Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext)
    {
        List<StateTransitionDesc> barriers;

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
                VertBuffDesc.uiSizeInBytes = resource.Vertices->GetSize();
                BufferData VBData;
                VBData.pData    = resource.Vertices->GetData();
                VBData.DataSize = resource.Vertices->GetSize();
                renderDevice->CreateBuffer(VertBuffDesc, &VBData, &resource.VertexBuffer);
                Log("Buffer created", ToString(resource.Vertices->GetCount()));
            } else {
                immediateContext->UpdateBuffer(resource.VertexBuffer, 0, resource.Vertices->GetSize(), resource.Vertices->GetData(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                barriers.push_back({resource.VertexBuffer, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_VERTEX_BUFFER, true});
            }

            if(resource.Indices.empty()) {
                continue;
            }

            if(resource.IndexBuffer == nullptr) {
                BufferDesc IndBuffDesc;
                IndBuffDesc.Name          = "Mesh index buffer";
                IndBuffDesc.Usage         = USAGE_IMMUTABLE;
                IndBuffDesc.BindFlags     = BIND_INDEX_BUFFER;
                IndBuffDesc.uiSizeInBytes = resource.Indices.size() * sizeof(uint32_t);
                BufferData IBData;
                IBData.pData    = resource.Indices.data();
                IBData.DataSize = resource.Indices.size() * sizeof(uint32_t);
                renderDevice->CreateBuffer(IndBuffDesc, &IBData, &resource.IndexBuffer);
                Log("Buffer created", ToString(resource.Indices.size()));
            } else {
                immediateContext->UpdateBuffer(resource.IndexBuffer, 0, resource.Indices.size() * sizeof(uint32_t), resource.Indices.data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                barriers.push_back({resource.IndexBuffer,  RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_INDEX_BUFFER,  true});
            }
        }

        immediateContext->TransitionResourceStates(barriers.size(), barriers.data());
    }
}