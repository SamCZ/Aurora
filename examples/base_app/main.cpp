#include <Aurora/Aurora.hpp>

#include <Aurora/Core/UUID.hpp>
#include <Aurora/Resource/ResourceManager.hpp>

#include <Aurora/Graphics/Material/MaterialDefinition.hpp>
#include <Aurora/Graphics/Material/Material.hpp>
#include <Aurora/Graphics/ViewPortManager.hpp>
#include <Aurora/Resource/MaterialLoader.hpp>

#include <Shaders/World/PBRBasic/cb_pbr.h>

#include <Aurora/Memory/Aum.hpp>
#include <Aurora/Framework/Scene.hpp>
#include <Aurora/Framework/Actor.hpp>
#include <Aurora/Framework/SceneComponent.hpp>
#include <Aurora/Framework/CameraComponent.hpp>
#include <Aurora/Framework/StaticMeshComponent.hpp>
#include <Aurora/Framework/Lights.hpp>

#include <Aurora/Resource/AssimpModelLoader.hpp>
#include <Aurora/Resource/ResourceName.hpp>

#include "Aurora/Editor/MainEditorPanel.hpp"

#include <Aurora/Render/VgRender.hpp>

#include <Aurora/Render/SceneRenderer.hpp>

using namespace Aurora;

class TestActor : public Actor
{
public:
	CLASS_OBJ(TestActor, Actor);

	TestActor()
	{
		std::cout << "TestActor created " << PointerToString(this) << std::endl;
	}

	void InitializeComponents() override
	{

	}

	void Tick(double delta) override
	{
		GetRootComponent()->GetTransform().Rotation.y += delta * 10.0f;
	}

	~TestActor() override
	{
		std::cout << "TestActor deleted" << std::endl;
	}
};

class CameraActor : public Actor
{
private:
	CameraComponent* m_Camera;
public:
	CLASS_OBJ(CameraActor, Actor);
	DEFAULT_COMPONENT(CameraComponent);

	void InitializeComponents() override
	{
		m_Camera = CameraComponent::Cast(GetRootComponent());
		m_Camera->SetName("Camera");
		m_Camera->SetViewPort(GEngine->GetViewPortManager()->Get());
		m_Camera->SetPerspective(75, 0.1f, 2000.0f);
	}

	void Tick(double delta) override
	{
		float m_FlySpeed = 10.0f;

		GetTransform().Rotation.x -= ImGui::GetIO().MouseDelta.y * 0.1f;
		GetTransform().Rotation.y -= ImGui::GetIO().MouseDelta.x * 0.1f;

		GetTransform().Rotation.x = glm::clamp(GetTransform().Rotation.x, -90.0f, 90.0f);
		GetTransform().Rotation.y = fmod(GetTransform().Rotation.y, 360.0f);

		Matrix4 transform = m_Camera->GetTransformationMatrix();

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_W)])
		{
			GetTransform().Location -= Vector3(transform[2]) * (float)delta * m_FlySpeed;
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_S)])
		{
			GetTransform().Location += Vector3(transform[2]) * (float)delta * m_FlySpeed;
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_A)])
		{
			GetTransform().Location -= Vector3(transform[0]) * (float)delta * m_FlySpeed;
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_D)])
		{
			GetTransform().Location += Vector3(transform[0]) * (float)delta * m_FlySpeed;
		}
	}

	CameraComponent* GetCamera()
	{
		return m_Camera;
	}
};

enum UI_TYPES
{
	UI_MAINMENU = 0,
	UI_SETTINGS = 1
};

class TestUI : public UserInterface
{
public:
	explicit TestUI(UIID_t id) : UserInterface(id) {}

	void BeginPlay() override
	{

	}

	void BeginDestroy() override
	{

	}

	void Tick(double delta) override
	{

	}
};

class TestGameMode : public GameModeBase
{
public:
	void BeginPlay() override
	{
		AddUserInterface<TestUI>(UI_MAINMENU);
	}

	void BeginDestroy() override
	{

	}

	void Tick(double delta) override
	{

	}
};

class BaseAppContext : public AppContext
{
	SceneRenderer* sceneRenderer;

	Actor* testActor = nullptr;

	~BaseAppContext() override
	{
		delete sceneRenderer;
	}

	void Init() override
	{
		SetGameContext<GameContext>();
		SwitchGameModeImmediately<TestGameMode>()->BeginPlay();

		sceneRenderer = new SceneRenderer();

		AssimpModelLoader modelLoader;
		MeshImportedData importedData = modelLoader.ImportModel("Test", GEngine->GetResourceManager()->LoadFile("Assets/socuwan.fbx"));

		testActor = GetScene()->SpawnActor<Actor>("TestActor", Vector3(0, 0, 0), {}, Vector3(0.01f));
		//CameraComponent* cameraComponent = actor->AddComponent<CameraComponent>("Camera");

		auto normalMap = GEngine->GetResourceManager()->LoadTexture("Assets/Textures/dry-rocky-ground-unity/dry-rocky-ground_normal-ogl.png");

		if (importedData)
		{
			auto matDef = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Textured.matd");

			auto* meshComponent = testActor->AddComponent<StaticMeshComponent>("Mesh");
			meshComponent->SetMesh(importedData.Mesh);

			for (auto &item : meshComponent->GetMaterialSet())
			{
				auto matInstance = matDef->CreateInstance();
				matInstance->SetTexture("Texture"_HASH, item.second.Textures["Diffuse"]);
				matInstance->SetTexture("NormalMap"_HASH, nullptr);
				item.second.Material = matInstance;
			}
		}

		MeshImportedData importedData2 = modelLoader.ImportModel("box", GEngine->GetResourceManager()->LoadFile("Assets/box_cubeUV.fbx"));
		auto matDef = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Textured.matd");
		auto matInstance = matDef->CreateInstance();
		matInstance->SetTexture("Texture"_HASH, GEngine->GetResourceManager()->LoadTexture("Assets/Textures/dry-rocky-ground-unity/dry-rocky-ground_albedo.png"));
		matInstance->SetTexture("NormalMap"_HASH, normalMap);

		for (int i = 0; i < 10; ++i)
		{
			Actor* testActor2 = GetScene()->SpawnActor<TestActor>("Box", Vector3(i * 2.2f, 0, -20), {}, Vector3(0.01f));

			if(importedData2)
			{
				auto* meshComponent = testActor2->AddComponent<StaticMeshComponent>("Mesh");
				meshComponent->SetMesh(importedData2.Mesh);

				for (auto &item : meshComponent->GetMaterialSet())
				{
					item.second.Material = matInstance;
				}
			}
		}

		GetScene()->SpawnActor<CameraActor>("Camera", {0, 0, 5});

		GetScene()->SpawnActor<PointLight>("PointLight", {-1, 3, 1});
	}

	void Update(double delta) override
	{

	}

	void Render() override
	{
		sceneRenderer->Render(GetScene());
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
	windowDefinition.Title = "Aurora - BaseApp";

	Aurora::AuroraEngine engine;
	engine.Init(new BaseAppContext(), windowDefinition, true);
	engine.Run();
	return 0;
}