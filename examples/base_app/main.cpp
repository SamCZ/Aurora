#include <Aurora/Aurora.hpp>
#include <Aurora/Resource/ResourceManager.hpp>

#include <Aurora/Graphics/Material/MaterialDefinition.hpp>
#include <Aurora/Graphics/Material/Material.hpp>
#include <Aurora/Resource/MaterialLoader.hpp>

#include <Shaders/World/PBRBasic/cb_pbr.h>

#include <Aurora/Memory/Aum.hpp>
#include <Aurora/Framework/Scene.hpp>
#include <Aurora/Framework/Actor.hpp>
#include <Aurora/Framework/SceneComponent.hpp>
#include <Aurora/Framework/CameraComponent.hpp>

#include <Aurora/Resource/AssimpModelLoader.hpp>

#include <imgui.h>
#include "Aurora/Editor/MainEditorPanel.hpp"

#include <Aurora/Graphics/OpenGL/GLRenderDevice.hpp>

#include <stb_image_write.h>

using namespace Aurora;

void HSVtoRGB(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV) {
	float fC = fV * fS; // Chroma
	float fHPrime = fmod(fH / 60.0, 6);
	float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
	float fM = fV - fC;

	if(0 <= fHPrime && fHPrime < 1) {
		fR = fC;
		fG = fX;
		fB = 0;
	} else if(1 <= fHPrime && fHPrime < 2) {
		fR = fX;
		fG = fC;
		fB = 0;
	} else if(2 <= fHPrime && fHPrime < 3) {
		fR = 0;
		fG = fC;
		fB = fX;
	} else if(3 <= fHPrime && fHPrime < 4) {
		fR = 0;
		fG = fX;
		fB = fC;
	} else if(4 <= fHPrime && fHPrime < 5) {
		fR = fX;
		fG = 0;
		fB = fC;
	} else if(5 <= fHPrime && fHPrime < 6) {
		fR = fC;
		fG = 0;
		fB = fX;
	} else {
		fR = 0;
		fG = 0;
		fB = 0;
	}

	fR += fM;
	fG += fM;
	fB += fM;
}

class TestActor : public Actor
{
private:
	int number;
	SceneComponent* m_Test;
public:
	CLASS_OBJ(TestActor, Actor);

	TestActor() : number(10)
	{
		std::cout << "TestActor created " << PointerToString(this) << std::endl;
	}

	void InitializeComponents() override
	{
		m_Test = AddComponent<SceneComponent>("Test");
		m_Test->GetTransform().Translation.x = 15;

		GetRootComponent()->GetTransform().Translation.x = 1000;
	}

	void Tick(double delta) override
	{
		std::cout << "tick" << std::endl;
	}

	void Test()
	{
		std::cout << "TestActor Test() " << number << " " << m_Test->GetTransform().Translation.x << " " << GetRootComponent()->GetTransform().Translation.x << std::endl;
	}

	~TestActor() override
	{
		std::cout << "TestActor deleted" << std::endl;
	}
};

class BaseAppContext : public AppContext
{
	MaterialDefinition_ptr matDef;
	std::shared_ptr<Material> mat;
	std::shared_ptr<Material> mat2;
	std::shared_ptr<Material> mat3;

	Scene scene;
	Mesh_ptr mesh = nullptr;

	MainEditorPanel mainEditorPanel;

	struct AOBakerVertex
	{
		Vector2 Pos;
		float Val;
	};
	MaterialDefinition_ptr matDefAO;
	std::shared_ptr<Material> matAO;
	Shader_ptr m_AOBakerShader;
	Buffer_ptr m_AOBakerVertexBuffer;
	InputLayout_ptr m_AOBakerInputLayout;

	void Init() override
	{
		matDef = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Test2D.matd");

		mat = matDef->CreateInstance();

		//mat->SetVariable("Color"_HASH, Vector4(0, 1, 0, 1));

		mat2 = mat->Clone();
		//mat2->SetVariable("Color"_HASH, Vector4(1, 1, 1, 1));

		mat2->SetTexture("Texture"_HASH, GEngine->GetResourceManager()->LoadTexture("Assets/Textures/logo_as.png", GraphicsFormat::RGBA8_UNORM, {}));

		AssimpModelLoader modelLoader;

		MeshImportedData importedData = modelLoader.ImportModel("Test", GEngine->GetResourceManager()->LoadFile("Assets/transform_test.fbx"));

		if(importedData)
		{
			mesh = importedData.Mesh;
		}

		TestActor* actor = scene.SpawnActor<TestActor>("", {0, 0, 0});
		CameraComponent* cameraComponent = actor->AddComponent<CameraComponent>("Camera");

		for(SceneComponent* component : scene.GetComponents<SceneComponent>())
		{
			std::cout << "Component " << component->GetName() << std::endl;
		}

		/////////////

		MaterialDefinitionDesc materialDefinitionDesc;
		materialDefinitionDesc.Name = "AOBaker";
		materialDefinitionDesc.Filepath = "[Internal]";
		materialDefinitionDesc.ShaderPasses[(PassType_t)EPassType::Ambient] = GEngine->GetResourceManager()->CreateShaderProgramDesc("AOBakerShader", {
			{EShaderType::Vertex, "Assets/Shaders/Utils/AOBaker.vert"},
			{EShaderType::Pixel, "Assets/Shaders/Utils/AOBaker.frag"}
		});

		matDefAO = std::make_shared<MaterialDefinition>(materialDefinitionDesc);
		matAO = matDefAO->CreateInstance();
		matAO->RasterState().CullMode = ECullMode::None;

		m_AOBakerVertexBuffer = GEngine->GetRenderDevice()->CreateBuffer(BufferDesc("AOBakerVB", sizeof(AOBakerVertex) * 8 * 3, EBufferType::VertexBuffer, EBufferUsage::DynamicDraw));
		m_AOBakerInputLayout = GEngine->GetRenderDevice()->CreateInputLayout({
			{"in_Pos", GraphicsFormat::RG32_FLOAT, 0, offsetof(AOBakerVertex, Pos), 0, sizeof(AOBakerVertex), false, false},
			{"in_Val", GraphicsFormat::R32_FLOAT, 0, offsetof(AOBakerVertex, Val), 1, sizeof(AOBakerVertex), false, false}
		});
	}

	float a = 0;

	void Update(double delta) override
	{
		//mainEditorPanel.Update();



		float fR = 0, fG = 0, fB = 0, fH = 0, fS = 0, fV = 0;

		fH = a * 255;
		fS = 0.19;
		fV = 255;

		HSVtoRGB(fR, fG, fB, fH, fS, fV);

		mat->SetVariable("Color"_HASH, Vector4(fR / 256.0f, fG / 256.0f, fB / 256.0f, 1));

		a += delta * 1.0f;
		if(a > 255) a = 0;
	}

	static constexpr int Index(int x, int y)
	{
		return x + y * 3;
	}

	void RenderAOSlice(DrawCallState& drawCallState, const bool blocks[8], int vpx, int vpy)
	{
		float unit = 256;
		float half = unit / 2.0f;

		AOBakerVertex* vertex = GEngine->GetRenderDevice()->MapBuffer<AOBakerVertex>(m_AOBakerVertexBuffer, EBufferAccess::WriteOnly);

		auto DrawQuadLB_RT = [&](float x, float y, float leftTop, float rightTop, float leftBottom, float rightBottom)
		{
			vertex->Pos = {x, y};
			vertex->Val = leftTop;
			vertex++;

			vertex->Pos = {x, y+ half};
			vertex->Val = leftBottom;
			vertex++;

			vertex->Pos = {x + half, y + half};
			vertex->Val = rightBottom;
			vertex++;

			vertex->Pos = {x, y};
			vertex->Val = leftTop;
			vertex++;

			vertex->Pos = {x + half, y + half};
			vertex->Val = rightBottom;
			vertex++;

			vertex->Pos = {x +half, y};
			vertex->Val = rightTop;
			vertex++;
		};

		auto DrawQuadLF_RB = [&](float x, float y, float leftTop, float rightTop, float leftBottom, float rightBottom)
		{
			vertex->Pos = {x + half, y};
			vertex->Val = rightTop;
			vertex++;

			vertex->Pos = {x, y};
			vertex->Val = leftTop;
			vertex++;

			vertex->Pos = {x, y + half};
			vertex->Val = leftBottom;
			vertex++;

			vertex->Pos = {x + half, y};
			vertex->Val = rightTop;
			vertex++;

			vertex->Pos = {x, y + half};
			vertex->Val = leftBottom;
			vertex++;

			vertex->Pos = {x + half, y + half};
			vertex->Val = rightBottom;
			vertex++;

		};

		bool leftTop = blocks[0];
		bool leftMiddle = blocks[1];
		bool leftBottom = blocks[2];

		bool midTop = blocks[7];
		bool midBottom = blocks[3];

		bool rightTop = blocks[6];
		bool rightMiddle = blocks[5];
		bool rightBottom = blocks[4];

		float defaultMiddleValue = 0.0f;

		DrawQuadLF_RB(0, 0,
			(leftTop || leftMiddle || midTop) * 0.75f,
			midTop * 0.5f,
			leftMiddle * 0.5f,
			defaultMiddleValue
		);
		DrawQuadLB_RT(half, 0,
			midTop * 0.5f,
			(midTop || rightTop || rightMiddle) * 0.75f,
			defaultMiddleValue,
			rightMiddle * 0.5f
		);

		DrawQuadLB_RT(0, half,
			leftMiddle * 0.5f,
			defaultMiddleValue,
			(leftMiddle || leftBottom || midBottom) * 0.75f,
			midBottom * 0.5f
		);

		DrawQuadLF_RB(half, half,
			defaultMiddleValue,
			rightMiddle * 0.5f,
			midBottom * 0.5f,
			(midBottom || rightBottom || rightMiddle) * 0.75f
		);

		GEngine->GetRenderDevice()->UnmapBuffer(m_AOBakerVertexBuffer);

		matAO->SetVariable("ProjectionMatrix"_HASH, glm::ortho(0.0f, (float)256, (float)256, 0.0f, 0.0f, 1.0f));

		drawCallState.PrimitiveType = EPrimitiveType::TriangleList;
		drawCallState.ViewPort = FViewPort(vpx, vpy, 256, 256);
		GEngine->GetRenderDevice()->SetViewPort(drawCallState.ViewPort);

		matAO->BeginPass((uint8)EPassType::Ambient ,drawCallState);

		drawCallState.InputLayoutHandle = m_AOBakerInputLayout;
		drawCallState.SetVertexBuffer(0, m_AOBakerVertexBuffer);
		GEngine->GetRenderDevice()->BindShaderInputs(drawCallState, true);

		GEngine->GetRenderDevice()->Draw(drawCallState, {DrawArguments(8 * 3)}, false);
		matAO->EndPass((uint8)EPassType::Ambient, drawCallState);
	}

	void Render() override
	{
		DrawCallState drawCallState;
		drawCallState.PrimitiveType = EPrimitiveType::TriangleStrip;
		drawCallState.ViewPort = FViewPort(GEngine->GetWindow()->GetSize());

		GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
		GEngine->GetRenderDevice()->ClearRenderTargets(drawCallState);

		{
			drawCallState.ViewPort = FViewPort(0, 0, 256, 256);
			GEngine->GetRenderDevice()->SetViewPort(drawCallState.ViewPort);

			mat->BeginPass((uint8)EPassType::Ambient ,drawCallState);
			GEngine->GetRenderDevice()->Draw(drawCallState, {DrawArguments(4)}, false);
			mat->EndPass((uint8)EPassType::Ambient, drawCallState);
		}

		{
			drawCallState.ViewPort = FViewPort(0, 256 + 16, 256, 256);
			GEngine->GetRenderDevice()->SetViewPort(drawCallState.ViewPort);

			mat2->BeginPass((uint8)EPassType::Ambient ,drawCallState);
			GEngine->GetRenderDevice()->Draw(drawCallState, {DrawArguments(4)}, false);
			mat2->EndPass((uint8)EPassType::Ambient, drawCallState);
		}

		{
			{
				static bool blocks[8]{false};
				static int permutationRemap[8] = {
					0, 7, 6,
					1, 5,
					2, 3, 4
				};
				bool middle = true;
				static int permutation = 0;

				TextureDesc textureDesc;
				textureDesc.Name = "AOBaked";
				textureDesc.ImageFormat = GraphicsFormat::RGBA8_UNORM;
				textureDesc.Width = 4096;
				textureDesc.Height = 4096;
				textureDesc.IsRenderTarget = true;
				textureDesc.MipLevels = 1;
				static Texture_ptr finalAO = GEngine->GetRenderDevice()->CreateTexture(textureDesc);

				if(ImGui::Begin("AO Baker"))
				{
					ImGui::Checkbox("##checkbox100", &blocks[0]); ImGui::SameLine();
					ImGui::Checkbox("##checkbox200", &blocks[7]); ImGui::SameLine();
					ImGui::Checkbox("##checkbox300", &blocks[6]);

					ImGui::Checkbox("##checkbox110", &blocks[1]); ImGui::SameLine();
					ImGui::Checkbox("##checkbox210", &middle); ImGui::SameLine();
					ImGui::Checkbox("##checkbox310", &blocks[5]);

					ImGui::Checkbox("##checkbox120", &blocks[2]); ImGui::SameLine();
					ImGui::Checkbox("##checkbox220", &blocks[3]); ImGui::SameLine();
					ImGui::Checkbox("##checkbox320", &blocks[4]);

					if(ImGui::DragInt("Permutation", &permutation, 1, 0, 255))
					{
						for (int i = 0; i < 8; ++i)
						{
							blocks[permutationRemap[i]] = (permutation & (1 << i)) > 0;
						}
					}

					if(ImGui::Button("Generate"))
					{
						drawCallState.BindTarget(0, finalAO);
						GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);

						for (int permutationCurrent = 0; permutationCurrent < 256; ++permutationCurrent)
						{
							for (int i = 0; i < 8; ++i)
							{
								blocks[permutationRemap[i]] = (permutationCurrent & (1 << i)) > 0;
							}

							int textureX = permutationCurrent % 16;
							int textureY = permutationCurrent / 16;

							RenderAOSlice(drawCallState, blocks, textureX * 256, textureY * 256);
						}

						mat2->SetTexture("Texture"_HASH, finalAO);

						GLTexture* glTexture = GetTexture(finalAO);
						glBindTexture(GL_TEXTURE_2D, glTexture->Handle());
						std::vector<uint8> imageData(4096 * 4096 * 4);
						glGetTexImage(GL_TEXTURE_2D,
							0,
							glTexture->Format().BaseFormat,
							glTexture->Format().Type,
							imageData.data());

						stbi_write_png("E:\\Sam2\\Sam\\Projects\\EmberSky\\client\\Assets\\Textures\\baked_ao.png", 4096, 4096, 4, imageData.data(), 4096 * 4);
					}

					drawCallState.ResetTargets();
					GEngine->GetRenderDevice()->BindRenderTargets(drawCallState);
				}
				ImGui::End();

				RenderAOSlice(drawCallState, blocks, 0, 256 + 256 + 16 + 16);
			}
		}
	}

	void RenderVg() override
	{

	}
};

int main()
{
	WindowDefinition windowDefinition = {};
	windowDefinition.Width = 1270;
	windowDefinition.Height = 720;
	windowDefinition.HasOSWindowBorder = true;
	windowDefinition.Maximized = true;
	windowDefinition.Title = "BaseApp";

	Aurora::AuroraEngine engine;
	engine.Init(new BaseAppContext(), windowDefinition, true);
	engine.Run();
	return 0;
}