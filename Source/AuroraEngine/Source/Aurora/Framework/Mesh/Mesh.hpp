#pragma once

#include <Aurora/Core/String.hpp>

#include "../../Render/VertexBuffer.hpp"
#include "../../Render/Material.hpp"

#include <Buffer.h>
#include <RenderDevice.h>
#include <RefCntAutoPtr.hpp>

namespace Aurora::Framework
{
    struct FMaterialSlot
    {
        Render::FMaterial* Material;
        String MaterialSlotName;

        FMaterialSlot() : Material(nullptr), MaterialSlotName(String_None) {}
        FMaterialSlot(Render::FMaterial* Material, String  materialSlotName) : Material(Material), MaterialSlotName(std::move(materialSlotName)) {}

        bool operator==(const FMaterialSlot& left) const { return Material == left.Material && MaterialSlotName == left.MaterialSlotName; }
    };

    struct FMeshSection
    {
        int32_t MaterialIndex;

        uint32_t FirstIndex;
        uint32_t NumTriangles;

        bool EnableCollision;
        bool CastShadow;

        FMeshSection()
                : MaterialIndex(0)
                , FirstIndex(0)
                , NumTriangles(0)
                , EnableCollision(false)
                , CastShadow(true)
        {
        }
    };

    struct HMeshLodResource
    {
        Render::IVertexBuffer* Vertices;
        List<uint32_t> Indices;
        List<FMeshSection> Sections;

        Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> IndexBuffer;
        bool NeedUpdateBuffers;
    };

    class HMesh
    {
    private:

    public:
        FastMap<uint8_t, HMeshLodResource> LODResources;
        FastMap<uint8_t, FMaterialSlot> MaterialSlots;

        void UpdateBuffers(Diligent::RefCntAutoPtr<Diligent::IRenderDevice>& renderDevice, Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& immediateContext);
    };
}