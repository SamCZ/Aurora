#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Render/RenderInterface.hpp"
#include <Aurora/Core/Vector.hpp>
#include <Aurora/Core/SmartPointer.hpp>

#include <EngineFactory.h>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>
#include <Timer.hpp>
#include <TextureLoader.h>
#include <TextureUtilities.h>

#include <RefCntAutoPtr.hpp>
#include <EngineFactoryVk.h>
#include <MapHelper.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "App/Window.hpp"
#include "App/Input/InputManager.hpp"


#include "Render/RenderInterface.hpp"
#include "Render/Material.hpp"
#include "../../../../External/DiligentEngine/DiligentTools/TextureLoader/interface/TextureLoader.h"

using namespace Diligent;
using namespace Aurora;
using namespace Aurora::App;
using namespace Aurora::Render;

static const char* VSSource = R"(
cbuffer Constants
{
    float4x4 rotato;
};
struct PSInput
{
    float4 Pos   : SV_POSITION;
    float2 TexCoord : TEXCOORD;
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

    float2 Tex[3];
    Tex[0] = float2(0.0, 0.0); // red
    Tex[1] = float2(0.0, 1.0); // green
    Tex[2] = float2(1.0, 1.0); // blue

    PSIn.Pos   = mul(rotato, Pos[VertId]);
    PSIn.Color = Col[VertId];
    PSIn.TexCoord = Tex[VertId];
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char* PSSource = R"(
struct PSInput
{
    float4 Pos   : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Color : COLOR;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

Texture2D    g_Texture;
SamplerState g_Texture_sampler; // By convention, texture samplers must use the '_sampler' suffix

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    //PSOut.Color = float4(PSIn.Color.rgb, 1.0);
    PSOut.Color = g_Texture.Sample(g_Texture_sampler, PSIn.TexCoord);
}
)";

static void run()
{
    Aurora::App::FWindowDefinition windowDefinition = {};
    windowDefinition.Width = 1280;
    windowDefinition.Height = 720;
    windowDefinition.HasOSWindowBorder = true;
    windowDefinition.Title = "Aurora Engine";

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

    FRenderInterface renderInterface(m_pDevice, m_pImmediateContext);

    TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = false;
    RefCntAutoPtr<ITexture> Tex;
    CreateTextureFromFile("DGLogo.png", loadInfo, m_pDevice, &Tex);

    if(Tex == nullptr) {
        std::cout << "Cannot load texture!" << std::endl;
    }


    FShaderCollectionPtr shaderCollection = std::make_shared<FShaderCollection>();

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
    shaderCollection->Vertex = pVS;

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint      = "main";
        ShaderCI.Desc.Name       = "Triangle pixel shader";
        ShaderCI.Source          = PSSource;
        m_pDevice->CreateShader(ShaderCI, &pPS);
    }
    shaderCollection->Pixel = pPS;

    FMaterial testMaterial("Triangel", shaderCollection);
    testMaterial.SetTexture("g_Texture", Tex);

    GraphicsPipelineDesc& graphicsPipelineDesc = testMaterial.GetPipelineDesc();
    graphicsPipelineDesc.NumRenderTargets = 1;
    graphicsPipelineDesc.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
    graphicsPipelineDesc.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;

    testMaterial.SetCullMode(CULL_MODE_NONE);
    testMaterial.SetDepthEnable(false);

    window.GetInputManager()->AddActionMapping("ToggleWireframe", Keys::M);

    window.GetInputManager()->BindAction("ToggleWireframe", IE_Pressed, new LambdaDelegate<void>([&testMaterial](){
        static bool state = true;
        testMaterial.SetFillMode(state ? FILL_MODE_WIREFRAME : FILL_MODE_SOLID);
        state = !state;
    }));

    float a = 0;

    Diligent::Timer timer;
    auto PrevTime          = timer.GetElapsedTime();
    double filteredFrameTime = 0.0;

    while(!window.IsShouldClose()) {
        auto CurrTime    = timer.GetElapsedTime();
        auto ElapsedTime = CurrTime - PrevTime;
        PrevTime         = CurrTime;

        glfwPollEvents();
        window.GetInputManager()->Update();

        auto* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
        auto* pDSV = m_pSwapChain->GetDepthBufferDSV();
        m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        testMaterial.ValidateGraphicsPipelineState(renderInterface);

        MapHelper<glm::mat4x4> CBConstants;
        if(testMaterial.GetConstantBuffer("Constants", CBConstants, m_pImmediateContext)) {
            CBConstants[0] = glm::rotate(a, glm::vec3(0, 0, 1));
        }

        a += 0.01f;

        // Clear the back buffer
        const float ClearColor[] = {0.350f, 0.350f, 0.350f, 1.0f};
        // Let the engine perform required state transitions
        m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        testMaterial.ApplyPipeline(renderInterface);
        //testMaterial.GetCurrentResourceBinding()->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
        testMaterial.CommitShaderResources(renderInterface);

        DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // Render 3 vertices
        m_pImmediateContext->Draw(drawAttrs);

        m_pSwapChain->Present(1);

        double filterScale = 0.2;
        filteredFrameTime  = filteredFrameTime * (1.0 - filterScale) + filterScale * ElapsedTime;
        std::stringstream fpsCounterSS;
        fpsCounterSS << "Aurora" << " - " << std::fixed << std::setprecision(1) << filteredFrameTime * 1000;
        fpsCounterSS << " ms (" << 1.0 / filteredFrameTime << " fps)";

        window.SetTitle(fpsCounterSS.str());
    }

    window.Destroy();

    glfwTerminate();
}