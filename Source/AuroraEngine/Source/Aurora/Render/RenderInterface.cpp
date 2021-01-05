#include "RenderInterface.hpp"

namespace Aurora::Render
{

    FRenderInterface::FRenderInterface(RefCntAutoPtr<IRenderDevice>& device, RefCntAutoPtr<IDeviceContext>& immediateContext) : m_Device(device), m_ImmediateContext(immediateContext)
    {

    }

    RefCntAutoPtr<ITexture> FRenderInterface::CreateRenderTarget2D(const String& name, const TEXTURE_FORMAT& format, int width, int height, const ColorRGBA& clearColor, int mipLevels, int sampleCount)
    {
        TextureDesc ColorDesc;
        ColorDesc.Name           = name.c_str();
        ColorDesc.Type           = RESOURCE_DIM_TEX_2D;
        ColorDesc.BindFlags      = BIND_RENDER_TARGET;
        ColorDesc.Width          = width;
        ColorDesc.Height         = height;
        ColorDesc.MipLevels      = mipLevels;
        ColorDesc.Format         = format;

        bool NeedsSRGBConversion = m_Device->GetDeviceCaps().IsD3DDevice() && (ColorDesc.Format == TEX_FORMAT_RGBA8_UNORM_SRGB || ColorDesc.Format == TEX_FORMAT_BGRA8_UNORM_SRGB);
        if (NeedsSRGBConversion)
        {
            // Internally Direct3D swap chain images are not SRGB, and ResolveSubresource
            // requires source and destination formats to match exactly or be typeless.
            // So we will have to create a typeless texture and use SRGB render target view with it.
            ColorDesc.Format = ColorDesc.Format == TEX_FORMAT_RGBA8_UNORM_SRGB ? TEX_FORMAT_RGBA8_TYPELESS : TEX_FORMAT_BGRA8_TYPELESS;
        }

        // Set the desired number of samples
        ColorDesc.SampleCount = sampleCount;
        // Define optimal clear value
        ColorDesc.ClearValue.Format   = format;
        ColorDesc.ClearValue.Color[0] = clearColor.r;
        ColorDesc.ClearValue.Color[1] = clearColor.g;
        ColorDesc.ClearValue.Color[2] = clearColor.b;
        ColorDesc.ClearValue.Color[3] = clearColor.a;
        RefCntAutoPtr<ITexture> pColor;
        m_Device->CreateTexture(ColorDesc, nullptr, &pColor);
        return pColor;
    }

    RefCntAutoPtr<ITexture> FRenderInterface::CreateRenderTarget2DDepth(const String& name, const TEXTURE_FORMAT& format, int width, int height, int sampleCount)
    {
        TextureDesc DepthDesc;
        DepthDesc.Name           = name.c_str();
        DepthDesc.Type           = RESOURCE_DIM_TEX_2D;
        DepthDesc.BindFlags      = BIND_DEPTH_STENCIL;
        DepthDesc.Width          = width;
        DepthDesc.Height         = height;
        DepthDesc.MipLevels      = 1;
        DepthDesc.Format         = format;
        DepthDesc.SampleCount = sampleCount;

        DepthDesc.ClearValue.Format               = DepthDesc.Format;
        DepthDesc.ClearValue.DepthStencil.Depth   = 1;
        DepthDesc.ClearValue.DepthStencil.Stencil = 0;

        RefCntAutoPtr<ITexture> pDepth;
        m_Device->CreateTexture(DepthDesc, nullptr, &pDepth);

        return pDepth;
    }

    RefCntAutoPtr<IRenderDevice>& FRenderInterface::GetDevice()
    {
        return m_Device;
    }

    RefCntAutoPtr<IDeviceContext>& FRenderInterface::GetImmediateContext()
    {
        return m_ImmediateContext;
    }
}