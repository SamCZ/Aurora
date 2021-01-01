#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Render/RenderInterface.hpp"
#include <Aurora/Core/Vector.hpp>

#include <EngineFactory.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>

#include <RefCntAutoPtr.hpp>
#include <EngineFactoryVk.h>
#include <MapHelper.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "App/Window.hpp"
#include "App/Input/InputManager.hpp"

using namespace Diligent;

static const char* VSSource = R"(
cbuffer Constants
{
    float4x4 rotato;
};
struct PSInput
{
    float4 Pos   : SV_POSITION;
    float3 Color : COLOR;
};

void main(in  uint    VertId : SV_VertexID,
          out PSInput PSIn)
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue

    PSIn.Pos   = mul(rotato, Pos[VertId]);
    PSIn.Color = Col[VertId];
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char* PSSource = R"(
struct PSInput
{
    float4 Pos   : SV_POSITION;
    float3 Color : COLOR;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = float4(PSIn.Color.rgb, 1.0);
}
)";

static void run()
{
    Aurora::App::FWindowDefinition windowDefinition = {};
    windowDefinition.Width = 1280;
    windowDefinition.Height = 720;
    windowDefinition.CenterScreen = true;
    windowDefinition.HasOSWindowBorder = true;

    Aurora::App::FWindow window;
    window.Initialize(windowDefinition, nullptr);
    window.Show();

    SwapChainDesc SCDesc;

#    if EXPLICITLY_LOAD_ENGINE_VK_DLL
    // Load the dll and import GetEngineFactoryVk() function
                auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#    endif
    EngineVkCreateInfo EngineCI;
#    ifdef DILIGENT_DEBUG
    EngineCI.EnableValidation = true;
#    endif

    RefCntAutoPtr<IRenderDevice>  m_pDevice;
    RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
    RefCntAutoPtr<ISwapChain>     m_pSwapChain;

    auto* pFactoryVk = GetEngineFactoryVk();
    pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_pDevice, &m_pImmediateContext);

    HWND hWnd = glfwGetWin32Window(window.GetWindowHandle());

    if (!m_pSwapChain && hWnd != nullptr)
    {
        Win32NativeWindow Window{hWnd};
        pFactoryVk->CreateSwapChainVk(m_pDevice, m_pImmediateContext, SCDesc, Window, &m_pSwapChain);
    }
    //LINUX
    //xcb_connection_t* connection = XGetXCBConnection(glfwGetX11Display());

    RefCntAutoPtr<IShaderResourceBinding> shaderResourceBinding;

    RefCntAutoPtr<IPipelineState> m_pPSO;
    RefCntAutoPtr<IBuffer> constants;

    BufferDesc CBDesc;
    CBDesc.Name           = "VS constants CB";
    CBDesc.uiSizeInBytes  = sizeof(Aurora::Matrix4);
    CBDesc.Usage          = USAGE_DYNAMIC;
    CBDesc.BindFlags      = BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    m_pDevice->CreateBuffer(CBDesc, nullptr, &constants);

    {
        // Pipeline state object encompasses configuration of all GPU stages

        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        PSOCreateInfo.PSODesc.Name = "Simple triangle PSO";

        // This is a graphics pipeline
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        // Set render target format which is the format of the swap chain's color buffer
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
        // Use the depth buffer format from the swap chain
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // No back face culling for this tutorial
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
        // Disable depth testing
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        // clang-format on

        ShaderCreateInfo ShaderCI;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        ShaderCI.UseCombinedTextureSamplers = true;
        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "Triangle vertex shader";
            ShaderCI.Source          = VSSource;
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        for (int i = 0; i < pVS->GetResourceCount(); ++i) {
            ShaderResourceDesc desc;
            pVS->GetResourceDesc(i, desc);

            std::cout << desc.Name << " " << desc.ArraySize << " " << desc.Type << std::endl;
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "Triangle pixel shader";
            ShaderCI.Source          = PSSource;
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        // Finally, create the pipeline state
        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;
        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);

        m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(constants);

        m_pPSO->CreateShaderResourceBinding(&shaderResourceBinding, true);

    }

    float a = 0;

    while(!window.IsShouldClose()) {
        glfwPollEvents();
        window.GetInputManager()->Update();

        auto* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
        auto* pDSV = m_pSwapChain->GetDepthBufferDSV();
        m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        MapHelper<glm::mat4x4> CBConstants(m_pImmediateContext, constants, MAP_WRITE, MAP_FLAG_DISCARD);
        CBConstants[0] = glm::rotate(a, glm::vec3(0, 0, 1));

        a += 0.01f;

        // Clear the back buffer
        const float ClearColor[] = {0.350f, 0.350f, 0.350f, 1.0f};
        // Let the engine perform required state transitions
        m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Set the pipeline state in the immediate context
        m_pImmediateContext->SetPipelineState(m_pPSO);

        m_pImmediateContext->CommitShaderResources(shaderResourceBinding, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // Render 3 vertices
        m_pImmediateContext->Draw(drawAttrs);

        m_pSwapChain->Present();
    }

    window.Destroy();

    glfwTerminate();
}