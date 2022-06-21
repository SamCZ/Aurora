#include <Aurora/Aurora.hpp>

#include <Aurora/Core/AUID.hpp>
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
#include <Aurora/Framework/SkeletalMeshComponent.hpp>
#include <Aurora/Framework/Lights.hpp>

#include <Aurora/Resource/AssimpModelLoader.hpp>
#include <Aurora/Resource/ResourceName.hpp>

#include "Aurora/Editor/MainEditorPanel.hpp"

#include <Aurora/Graphics/VgRender.hpp>

#include <Aurora/Render/SceneRendererDeferred.hpp>
#include <Aurora/Render/SceneRendererForward.hpp>
#include <Aurora/Render/SceneRendererDeferredNew.hpp>

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
		GetRootComponent()->GetTransform().AddRotation(0, delta * 10.0f, 0);
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

		//GetTransform().Rotation.x += delta;

		/*GetTransform().Rotation.x -= ImGui::GetIO().MouseDelta.y * 0.1f;
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
		}*/
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

	// FIXME: this is just an hack!
	SceneRenderer* GetSceneRenderer() override { return sceneRenderer; }

	void Init() override
	{
		SetGameContext<GameContext>();
		SwitchGameModeImmediately<TestGameMode>()->BeginPlay();

		sceneRenderer = new SceneRendererForward();

		AssimpModelLoader modelLoader;
		MeshImportedData importedData = modelLoader.ImportModel("Test", GEngine->GetResourceManager()->LoadFile("Assets/socuwan.fbx"));

		testActor = GetScene()->SpawnActor<Actor, StaticMeshComponent>("TestActor", Vector3(0, 0, 0), {}, Vector3(0.01f));
		//CameraComponent* cameraComponent = actor->AddComponent<CameraComponent>("Camera");

		auto normalMap = GEngine->GetResourceManager()->LoadTexture("Assets/Textures/dry-rocky-ground-unity/dry-rocky-ground_normal-ogl.png");

		if (importedData)
		{
			auto matDef = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Textured.matd");

			auto* meshComponent = StaticMeshComponent::Cast(testActor->GetRootComponent());
			meshComponent->SetMesh(importedData.Mesh);

			for (auto &item : meshComponent->GetMaterialSet())
			{
				auto matInstance = matDef->CreateInstance();
				matInstance->SetTexture("Texture"_HASH, item.second.Textures["Diffuse"]);

				matInstance->RasterState(0).CullMode = ECullMode::None;
				matInstance->RasterState(1).CullMode = ECullMode::None;

				if (item.second.Textures.contains("Normal"))
				{
					matInstance->SetTexture("NormalMap"_HASH, item.second.Textures["Normal"]);
				}
				else
				{
					matInstance->SetTexture("NormalMap"_HASH, nullptr);
				}
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
			Actor* testActor2 = GetScene()->SpawnActor<TestActor, StaticMeshComponent>("Box", Vector3(i * 2.2f, 0, -20), {}, Vector3(0.01f));

			if(importedData2)
			{
				auto* meshComponent = StaticMeshComponent::Cast(testActor2->GetRootComponent());
				meshComponent->SetMesh(importedData2.Mesh);

				for (auto &item : meshComponent->GetMaterialSet())
				{
					item.second.Material = matInstance;
				}
			}
		}

		GetScene()->SpawnActor<CameraActor>("Camera", {0, 0, 5});

		GetScene()->SpawnActor<PointLight>("PointLight", {-1, 3, 1});

		{ // Animation Test
			MeshImportedData animImport = modelLoader.ImportModel("hands", GEngine->GetResourceManager()->LoadFile("Assets/Aim/Pickaxe_swing_animation.fbx"));

			auto skinnedMat = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Skinned.matd")->CreateInstance();

			if (animImport == true)
			{
				SkeletalMesh_ptr skeletalMesh = animImport.Get<SkeletalMesh>();

				//modelLoader.ImportAnimation(GEngine->GetResourceManager()->LoadFile("Assets/Aim/Pupper - anim.fbx"), skeletalMesh);
				//modelLoader.ImportAnimation(GEngine->GetResourceManager()->LoadFile("Assets/Aim/jump.fbx"), skeletalMesh);
				//modelLoader.ImportAnimation(GEngine->GetResourceManager()->LoadFile("Assets/Aim/walk.fbx"), skeletalMesh);

				SkeletalMeshComponent* skeletalMeshComponent = GetScene()->SpawnActor<Actor, SkeletalMeshComponent>("AnimationTest")->GetRootComponent<SkeletalMeshComponent>();
				skeletalMeshComponent->SetMesh(skeletalMesh);
				skeletalMeshComponent->SetIgnoreFrustumChecks(true);

				//skeletalMeshComponent->GetTransform().SetLocation(-animImport.Mesh->m_Bounds.GetOrigin());

				AU_LOG_INFO(glm::to_string(animImport.Mesh->m_Bounds.GetOrigin()));
				AU_LOG_INFO(glm::to_string(animImport.Mesh->m_Bounds.GetSize()));

				for (auto &item : skeletalMeshComponent->GetMaterialSet())
				{
					item.second.Material = skinnedMat;
				}
			}
		}
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