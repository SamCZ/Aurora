#include <iostream>

#include <Aurora/Aurora.hpp>
#include <Aurora/AuroraEngine.hpp>

#include <MapHelper.hpp>

#include <Aurora/Render/Material.hpp>

#include <Aurora/Assets/ModelImporter.hpp>

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

static const char* VSSourceMesh = R"(
cbuffer Constants
{
    float4x4 MVP;
};

struct VS_Input
{
	float3 position : ATTRIB0;
	float2 texCoord : ATTRIB1;

	float3 normal : ATTRIB2;
	float3 tangent : ATTRIB3;
	float3 bitan : ATTRIB4;
};

struct PSInput
{
    float4 Pos   : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Color : COLOR;
};

void main(in VS_Input input, out PSInput PSIn)
{
    PSIn.Pos   = mul(MVP, float4(input.position, 1.0));
    //PSIn.Color = input.normal;
    PSIn.TexCoord = input.texCoord;
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
    HStaticMeshPtr staticMesh;

    double a = 0;
public:
    explicit TestGame(FWindowPtr window) : FWindowGameContext(std::move(window)) {}

    void Init() override
    {
        staticMesh = FModelImporter::LoadMesh(AuroraEngine::AssetManager->LoadFile("sword_pbr.fbx"));
        staticMesh->UpdateBuffers(AuroraEngine::RenderDevice, AuroraEngine::ImmediateContext);

        Tex = AuroraEngine::AssetManager->LoadTexture("BaseColor.png");

        if(Tex == nullptr) {
            std::cout << "Cannot load texture!" << std::endl;
        }

        FShaderCollectionPtr shaders = AuroraEngine::AssetManager->LoadShaders({
            {SHADER_TYPE_VERTEX, SHADER_SOURCE_LANGUAGE_HLSL, "", VSSourceMesh, {}},
            {SHADER_TYPE_PIXEL, SHADER_SOURCE_LANGUAGE_HLSL, "", PSSource, {}}
        });

        testMaterial = New(FMaterial, "Tringel", shaders);
        testMaterial->SetTexture("g_Texture", Tex);

        GraphicsPipelineDesc& graphicsPipelineDesc = testMaterial->GetPipelineDesc();
        graphicsPipelineDesc.NumRenderTargets = 1;
        graphicsPipelineDesc.RTVFormats[0]                = GetSwapChain()->GetDesc().ColorBufferFormat;
        graphicsPipelineDesc.DSVFormat                    = GetSwapChain()->GetDesc().DepthBufferFormat;

        testMaterial->SetCullMode(CULL_MODE_NONE);
        testMaterial->SetDepthEnable(true);
        testMaterial->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        graphicsPipelineDesc.InputLayout.LayoutElements = staticMesh->GetLayout();
        graphicsPipelineDesc.InputLayout.NumElements = staticMesh->GetLayoutElementCount();

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

        Uint32   offset   = 0;
        IBuffer* pBuffs[] = {staticMesh->LODResources[0].VertexBuffer};
        AuroraEngine::ImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        AuroraEngine::ImmediateContext->SetIndexBuffer(staticMesh->LODResources[0].IndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        testMaterial->ValidateGraphicsPipelineState();

        MapHelper<glm::mat4x4> CBConstants;
        if(testMaterial->GetConstantBuffer("Constants", CBConstants)) {
            float aspect = (float)1280 / (float)720;
            float h = glm::tan(glm::radians(90.0f) * .5f) * 0.01f;
            float w = h * aspect;
            float _FrustumLeft = -w;
            float _FrustumRight = w;
            float _FrustumBottom = -h;
            float _FrustumTop = h;
            float _FrustumNear = 0.01f;
            float _FrustumFar = 1000;

            Matrix4 proj = glm::frustum(_FrustumLeft, _FrustumRight, _FrustumBottom, _FrustumTop, _FrustumNear, _FrustumFar);

            CBConstants[0] = proj * glm::translate(Vector3(0, 0, -10)) * glm::rotate((float)a, Vector3(0, 1, 0)) * glm::scale(Vector3(0.01f));
        }

        testMaterial->ApplyPipeline();
        testMaterial->CommitShaderResources();

        for (auto& section : staticMesh->LODResources[0].Sections) {
            DrawIndexedAttribs drawAttrs;
            drawAttrs.IndexType  = VT_UINT32;
            drawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
            drawAttrs.FirstIndexLocation = section.FirstIndex;
            drawAttrs.NumIndices = section.NumTriangles;
            AuroraEngine::ImmediateContext->DrawIndexed(drawAttrs);
        }
    }
};

int main()
{
    AuroraEngine::Init();
    AuroraEngine::AddWindow<TestGame>(1280, 720, "EmberSky");
    return AuroraEngine::Run();
}