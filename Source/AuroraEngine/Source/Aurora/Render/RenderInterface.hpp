#pragma once

#include <PipelineState.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <RefCntAutoPtr.hpp>

#include <Aurora/Core/ColorRGBA.hpp>

using namespace Diligent;

namespace Aurora::Render
{
    class FRenderInterface
    {
    private:
        RefCntAutoPtr<IRenderDevice> m_Device;
        RefCntAutoPtr<IDeviceContext> m_ImmediateContext;
    public:
        FRenderInterface(RefCntAutoPtr<IRenderDevice>& device, RefCntAutoPtr<IDeviceContext>& immediateContext);
        ~FRenderInterface() = default;
    public:

        RefCntAutoPtr<ITexture> CreateRenderTarget2D(const String& name, const TEXTURE_FORMAT& format, int width, int height, const ColorRGBA& clearColor, int mipLevels = 1, int sampleCount = 1);
        RefCntAutoPtr<ITexture> CreateRenderTarget2DDepth(const String& name, const TEXTURE_FORMAT& format, int width, int height, int sampleCount = 1);

    public:
        RefCntAutoPtr<IRenderDevice>& GetDevice();
        RefCntAutoPtr<IDeviceContext>& GetImmediateContext();
    };
}