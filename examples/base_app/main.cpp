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
		//m_Test->GetTransform().Location.x = 15;

		//GetRootComponent()->GetTransform().Location.x = 1000;
	}

	void Tick(double delta) override
	{
		std::cout << "tick" << std::endl;
	}

	void Test()
	{
		std::cout << "TestActor Test() " << number << " " << m_Test->GetTransform().Location.x << " " << GetRootComponent()->GetTransform().Location.x << std::endl;
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
	MainEditorPanel* mainEditorPanel;
	SceneRenderer* sceneRenderer;

	Actor* testActor;
	Actor* testActor2;

	~BaseAppContext() override
	{
		delete sceneRenderer;
		delete mainEditorPanel;
	}

	void Init() override
	{
		SetGameContext<GameContext>();
		SwitchGameMode<TestGameMode>();

		mainEditorPanel = new MainEditorPanel();
		sceneRenderer = new SceneRenderer();

		AssimpModelLoader modelLoader;
		MeshImportedData importedData = modelLoader.ImportModel("Test", GEngine->GetResourceManager()->LoadFile("Assets/sponza.fbx"));

		testActor = GetScene().SpawnActor<TestActor>("TestActor", Vector3(0, 0, 0), {}, Vector3(0.0001f));
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

		testActor2 = GetScene().SpawnActor<Actor>("Box", Vector3(0, 0, 0), {}, Vector3(0.01f));
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

		GetScene().SpawnActor<CameraActor>("Camera", {0, 0, 5});
	}

	void Update(double delta) override
	{
		mainEditorPanel->Update(delta);

		//testActor->GetRootComponent()->GetTransform().Rotation.x += delta * 50.0f;
		//testActor->GetRootComponent()->GetTransform().Rotation.y += delta * 50.0f;

		testActor2->GetRootComponent()->GetTransform().Location.x = 10;
		testActor2->GetRootComponent()->GetTransform().Rotation.y += delta * 20.0f;
	}

	void Render() override
	{
		sceneRenderer->Render(&GetScene());
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