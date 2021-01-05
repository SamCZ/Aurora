#include <iostream>

#include <Aurora/Aurora.hpp>
#include <Aurora/AuroraEngine.hpp>

#include <TextureLoader.h>
#include <TextureUtilities.h>
#include <MapHelper.hpp>

#include <Aurora/Render/Material.hpp>

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

class TestGame : public FWindowGameContext
{
private:
    RefCntAutoPtr<ITexture> Tex;
    FMaterialPtr testMaterial;

    double a = 0;
public:
    explicit TestGame(FWindowPtr window) : FWindowGameContext(std::move(window)) {}

    void Init() override
    {
        TextureLoadInfo loadInfo;
        loadInfo.IsSRGB = false;
        CreateTextureFromFile("DGLogo.png", loadInfo, AuroraEngine::RenderDevice, &Tex);

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
            AuroraEngine::RenderDevice->CreateShader(ShaderCI, &pVS);
        }
        shaderCollection->Vertex = pVS;

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "Triangle pixel shader";
            ShaderCI.Source          = PSSource;
            AuroraEngine::RenderDevice->CreateShader(ShaderCI, &pPS);
        }
        shaderCollection->Pixel = pPS;

        testMaterial = New(FMaterial, "Tringel", shaderCollection);
        testMaterial->SetTexture("g_Texture", Tex);

        GraphicsPipelineDesc& graphicsPipelineDesc = testMaterial->GetPipelineDesc();
        graphicsPipelineDesc.NumRenderTargets = 1;
        graphicsPipelineDesc.RTVFormats[0]                = GetSwapChain()->GetDesc().ColorBufferFormat;
        graphicsPipelineDesc.DSVFormat                    = GetSwapChain()->GetDesc().DepthBufferFormat;

        testMaterial->SetCullMode(CULL_MODE_NONE);
        testMaterial->SetDepthEnable(false);

        GetWindow()->GetInputManager()->AddActionMapping("ToggleWireframe", Keys::M);

        GetWindow()->GetInputManager()->BindAction("ToggleWireframe", IE_Pressed, new LambdaDelegate<void>([this](){
            static bool state = true;
            testMaterial->SetFillMode(state ? FILL_MODE_WIREFRAME : FILL_MODE_SOLID);
            state = !state;
        }));
    }

    void Update(double delta, double currentTime) override
    {
        a += 1.0 * delta;
    }

    void Render() override
    {
        auto* pRTV = GetSwapChain()->GetCurrentBackBufferRTV();
        auto* pDSV = GetSwapChain()->GetDepthBufferDSV();
        AuroraEngine::ImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Clear the back buffer
        const float ClearColor[] = {0.350f, 0.350f, 0.350f, 1.0f};
        // Let the engine perform required state transitions
        AuroraEngine::ImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        AuroraEngine::ImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        testMaterial->ValidateGraphicsPipelineState();

        MapHelper<glm::mat4x4> CBConstants;
        if(testMaterial->GetConstantBuffer("Constants", CBConstants)) {
            CBConstants[0] = glm::rotate((float)a, glm::vec3(0, 0, 1));
        }

        testMaterial->ApplyPipeline();
        testMaterial->CommitShaderResources();

        DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // Render 3 vertices
        AuroraEngine::ImmediateContext->Draw(drawAttrs);
    }
};

int main()
{
    AuroraEngine::Init();
    AuroraEngine::AddWindow<TestGame>(1280, 720, "EmberSky");
    return AuroraEngine::Run();
}