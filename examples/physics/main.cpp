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
#include <Aurora/Framework/Lights.hpp>

#include <Aurora/Resource/AssimpModelLoader.hpp>
#include <Aurora/Resource/ResourceName.hpp>

#include "Aurora/Editor/MainEditorPanel.hpp"

#include <Aurora/Graphics/VgRender.hpp>

#include <Aurora/Render/SceneRenderer.hpp>

#include <Aurora/Physics/PhysicsWorld.hpp>
#include <Aurora/Framework/Physics/RigidBodyComponent.hpp>
#include <Aurora/Framework/Physics/ColliderComponent.hpp>

using namespace Aurora;

class CameraActor : public Actor
{
private:
	CameraComponent* m_Camera;
public:
	CLASS_OBJ(CameraActor, Actor);
	DEFAULT_COMPONENT(CameraComponent);

	RigidBodyComponent* rigidBody;

	void InitializeComponents() override
	{
		m_Camera = CameraComponent::Cast(GetRootComponent());
		m_Camera->SetName("Camera");
		m_Camera->SetViewPort(GEngine->GetViewPortManager()->Get());
		m_Camera->SetPerspective(75, 0.1f, 2000.0f);

		rigidBody = AddComponent<RigidBodyComponent>();

		AddComponent<BoxColliderComponent>(1, 1, 1);
	}

	void Tick(double delta) override
	{
		//GetTransform().Rotation.x += delta;

		float yaw = ImGui::GetIO().MouseDelta.y * -0.1f;
		float pitch = ImGui::GetIO().MouseDelta.x * -0.1f;
		m_Camera->GetTransform().AddRotation(yaw, pitch, 0.0f);
	}

	void FixedStep() override
	{
		float m_FlySpeed = 6.0f;

		Matrix4 transform = m_Camera->GetTransformationMatrix();

		bool hadAccY = rigidBody->GetAcceleration().y > 0;

		rigidBody->SetVelocity({0, 0, 0});
		rigidBody->SetAcceleration({0, 0, 0});

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_W)])
		{
			//m_Camera->GetTransform().AddLocation(-Vector3(transform[2]) * (float)delta * m_FlySpeed);
			rigidBody->AddVelocity(-Vector3(transform[2]) * m_FlySpeed);
			//rigidBody->SetLinearVelocity();
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_S)])
		{
			//m_Camera->GetTransform().AddLocation(Vector3(transform[2]) * (float)delta * m_FlySpeed);
			rigidBody->AddVelocity(Vector3(transform[2]) * m_FlySpeed);
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_A)])
		{
			//m_Camera->GetTransform().AddLocation(-Vector3(transform[0]) * (float)delta * m_FlySpeed);
			rigidBody->AddVelocity(-Vector3(transform[0]) * m_FlySpeed);
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_D)])
		{
			//m_Camera->GetTransform().AddLocation(Vector3(transform[0]) * (float)delta * m_FlySpeed);
			rigidBody->AddVelocity(Vector3(transform[0]) * m_FlySpeed);
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_Space)] && not hadAccY)
		{
			//m_Camera->GetTransform().AddLocation(Vector3(transform[0]) * (float)delta * m_FlySpeed);
			rigidBody->SetAcceleration({0, 40, 0});
		}

		//rigidBody->SetVelocity(rigidBody->GetVelocity() * 0.99f);
	}

	CameraComponent* GetCamera()
	{
		return m_Camera;
	}
};

class BaseAppContext : public AppContext
{
	SceneRenderer* sceneRenderer;

	Actor* testActor = nullptr;

	PhysicsWorld* m_PhysicsWorld;

	~BaseAppContext() override
	{
		delete m_PhysicsWorld;
		delete sceneRenderer;
	}

	// FIXME: this is just an hack!
	SceneRenderer* GetSceneRenderer() override { return sceneRenderer; }
	PhysicsWorld* GetPhysicsWorld() override { return m_PhysicsWorld; }

	void Init() override
	{
		SetGameContext<GameContext>();
		SwitchGameModeImmediately<GameModeBase>()->BeginPlay();

		m_PhysicsWorld = new PhysicsWorld(GetScene());

		sceneRenderer = new SceneRenderer();

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

		if(false)
		{
			Actor* groundActor = GetScene()->SpawnActor<Actor>("Ground", {0, -0.5f, 0});
			groundActor->AddComponent<BoxColliderComponent>(50, 1, 50);
		}
		else
		{
			for (int x = -10; x < 10; ++x)
			{
				for (int z = -10; z < 10; ++z)
				{
					Actor* groundActor = GetScene()->SpawnActor<Actor>("Ground", {x + 0.5f, -0.5f, z + 0.5f});
					groundActor->AddComponent<BoxColliderComponent>(1, 1, 1);
				}
			}
		}

		MeshImportedData importedData2 = modelLoader.ImportModel("box", GEngine->GetResourceManager()->LoadFile("Assets/box_cubeUV.fbx"));
		auto matDef = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Textured.matd");
		auto matInstance = matDef->CreateInstance();
		matInstance->SetTexture("Texture"_HASH, GEngine->GetResourceManager()->LoadTexture("Assets/Textures/dry-rocky-ground-unity/dry-rocky-ground_albedo.png"));
		matInstance->SetTexture("NormalMap"_HASH, normalMap);

		for (int i = 0; i < 10; ++i)
		{
			Actor* testActor2 = GetScene()->SpawnActor<Actor, StaticMeshComponent>("Box " + std::to_string(i), Vector3(i * 2.2f - 5, 10, 0), {}, Vector3(0.005f));

			//RigidBodyComponent* rigidBodyComponent = testActor2->AddComponent<RigidBodyComponent>();
			//BoxColliderComponent* collider = testActor2->AddComponent<BoxColliderComponent>(1, 1, 1);

			if(importedData2)
			{
				auto* meshComponent = StaticMeshComponent::Cast(testActor2->GetRootComponent());
				meshComponent->SetMesh(importedData2.Mesh);

				for (auto &item : meshComponent->GetMaterialSet())
				{
					item.second.Material = matInstance;
				}
			}

			//break;
		}

		for (int i = 0; i < 10; ++i)
		{
			Actor* testActor2 = GetScene()->SpawnActor<Actor, StaticMeshComponent>("Box B " + std::to_string(i), Vector3(i * 2.2f - 5, 10 + i + 1, 0), {}, Vector3(0.005f));

			//BoxColliderComponent* collider = testActor2->AddComponent<BoxColliderComponent>(1, 1, 1);

			if (i != 9)
			{
				//RigidBodyComponent* rigidBodyComponent = testActor2->AddComponent<RigidBodyComponent>();
			}
			else
			{
				BoxColliderComponent* collider = testActor2->AddComponent<BoxColliderComponent>(1, 1, 1);
				testActor->SetName("Static box");
				testActor2->GetTransform().SetLocation(0, 0, 0);
			}

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

		GetScene()->SpawnActor<CameraActor>("Camera", {0, 3, 5});

		GetScene()->SpawnActor<PointLight>("PointLight", {-1, 3, 1});
	}

	void Update(double delta) override
	{
		m_PhysicsWorld->Update(delta);
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