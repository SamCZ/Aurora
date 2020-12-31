#pragma once

#include <PipelineState.h>

namespace Aurora::Render
{
    class RenderInterface
    {
    public:
        static uint32_t GetGraphicsPipelineStateCreateInfoHash(Diligent::GraphicsPipelineStateCreateInfo& gpsci);
    };
}