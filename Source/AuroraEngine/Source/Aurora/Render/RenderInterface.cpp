#include "RenderInterface.hpp"
#include <Aurora/Core/Crc.hpp>

namespace Aurora::Render
{

    uint32_t RenderInterface::GetGraphicsPipelineStateCreateInfoHash(Diligent::GraphicsPipelineStateCreateInfo& gpsci)
    {
        CrcHash hasher;

        Diligent::GraphicsPipelineDesc& gpd = gpsci.GraphicsPipeline;

        // Hash blend state
        hasher.Add(gpd.BlendDesc);

        // Hash raster state
        hasher.Add(gpd.RasterizerDesc);

        // Hash depth stencil
        hasher.Add(gpd.DepthStencilDesc);

        // Hash input layout
        Diligent::InputLayoutDesc& inputLayout = gpd.InputLayout;
        hasher.Add(inputLayout.NumElements);
        for (int i = 0; i < inputLayout.NumElements; ++i) {
            hasher.Add(inputLayout.LayoutElements[i]);
        }

        // Hash other things
        hasher.Add(gpd.PrimitiveTopology);
        hasher.Add(gpd.NumViewports);
        hasher.Add(gpd.NumRenderTargets);
        hasher.Add(gpd.SubpassIndex);
        hasher.Add(gpd.SubpassIndex);
        hasher.Add(gpd.SmplDesc);

        // Hash shaders

        hasher.Add(gpsci.pVS);
        hasher.Add(gpsci.pPS);
        hasher.Add(gpsci.pDS);
        hasher.Add(gpsci.pHS);
        hasher.Add(gpsci.pGS);
        hasher.Add(gpsci.pAS);
        hasher.Add(gpsci.pMS);

        return hasher.Get();
    }
}