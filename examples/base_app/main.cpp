#include <Aurora/Aurora.hpp>
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

#include <Aurora/Resource/AssimpModelLoader.hpp>

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
		GetRootComponent()->GetTransform().Location.x = 10;
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
		m_Camera->SetViewPort(GEngine->GetViewPortManager()->Get());
		m_Camera->SetPerspective(75, 0.1f, 2000.0f);
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
	Actor* testActor2 = nullptr;

	~BaseAppContext() override
	{
		delete sceneRenderer;
	}

	void Init() override
	{
		SetGameContext<GameContext>();
		SwitchGameMode<TestGameMode>();

		sceneRenderer = new SceneRenderer();

		AssimpModelLoader modelLoader;
		MeshImportedData importedData = modelLoader.ImportModel("Test", GEngine->GetResourceManager()->LoadFile("Assets/sponza.fbx"));

		testActor = GetScene()->SpawnActor<Actor>("TestActor", Vector3(0, 0, 0), {}, Vector3(0.0001f));
		//CameraComponent* cameraComponent = actor->AddComponent<CameraComponent>("Camera");

		if (importedData)
		{
			auto matDef = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Textured.matd");

			auto* meshComponent = testActor->AddComponent<StaticMeshComponent>("Mesh");
			meshComponent->SetMesh(importedData.Mesh);

			for (auto &item : meshComponent->GetMaterialSet())
			{
				auto matInstance = matDef->CreateInstance();
				matInstance->SetTexture("Texture"_HASH, item.second.Textures["Diffuse"]);
				item.second.Material = matInstance;
			}
		}

		testActor2 = GetScene()->SpawnActor<TestActor>("Box", Vector3(0, 0, 0), {}, Vector3(0.01f));
		MeshImportedData importedData2 = modelLoader.ImportModel("box", GEngine->GetResourceManager()->LoadFile("Assets/box.fbx"));
		if(importedData2)
		{
			auto matDef = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Color.matd");

			auto* meshComponent = testActor2->AddComponent<StaticMeshComponent>("Mesh");
			meshComponent->SetMesh(importedData2.Mesh);

			auto matInstance = matDef->CreateInstance();

			for (auto &item : meshComponent->GetMaterialSet())
			{
				//matInstance->SetTexture("Texture"_HASH, item.second.Textures["Diffuse"]);
				item.second.Material = matInstance;
			}
		}

		GetScene()->SpawnActor<CameraActor>("Camera", {0, 0, 5});
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
	windowDefinition.Title = "BaseApp";

	Aurora::AuroraEngine engine;
	engine.Init(new BaseAppContext(), windowDefinition, true);
	engine.Run();
	return 0;
}