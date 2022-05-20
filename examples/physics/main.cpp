#include <Aurora/Aurora.hpp>

#include <Aurora/Core/AUID.hpp>
#include <Aurora/Resource/ResourceManager.hpp>

#include <Aurora/Graphics/Material/MaterialDefinition.hpp>
#include <Aurora/Graphics/Material/Material.hpp>
#include <Aurora/Graphics/ViewPortManager.hpp>
#include <Aurora/Graphics/DShape.hpp>
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

#include <Aurora/Render/SceneRendererDeferred.hpp>

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
	BoxColliderComponent* collider;

	void InitializeComponents() override
	{
		m_Camera = CameraComponent::Cast(GetRootComponent());
		m_Camera->SetName("Camera");
		m_Camera->SetViewPort(GEngine->GetViewPortManager()->Get());
		m_Camera->SetPerspective(75, 0.1f, 2000.0f);

		rigidBody = AddComponent<RigidBodyComponent>();
		rigidBody->SetFriction(0.80f);

		collider = AddComponent<BoxColliderComponent>(0.75f, 1.75f, 0.75f);
		collider->SetOrigin({0, -0.7f, 0});
	}

	void Tick(double delta) override
	{
		//GetTransform().Rotation.x += delta;

		DShapes::Box(collider->GetTransformedAABB(), Color::blue(), true);

		float yaw = ImGui::GetIO().MouseDelta.y * -0.1f;
		float pitch = ImGui::GetIO().MouseDelta.x * -0.1f;
		m_Camera->GetTransform().AddRotation(yaw, pitch, 0.0f);

		bool isOnGround = rigidBody->GetVelocity().y >= 0 && rigidBody->GetVelocity().y < 0.05f;

		if(ImGui::IsKeyPressed(ImGuiKey_Space, false) && isOnGround)
		{
			rigidBody->AddAcceleration({0, 10, 0});
		}
	}

	void FixedStep() override
	{
		float speed = 1.0f;

		Matrix4 transform = m_Camera->GetTransformationMatrix();
		Vector3 forward = -glm::normalize(Vector3(transform[2])) * speed;
		Vector3 left = -glm::normalize(Vector3(transform[0])) * speed;

		forward.y = 0;
		left.y = 0;

		//AU_LOG_INFO(glm::to_string(rigidBody->GetVelocity()));

		bool isOnGround = rigidBody->GetVelocity().y >= 0 && rigidBody->GetVelocity().y < 0.05f;

		if (not isOnGround)
		{
			forward *= 0.5f;
			left *= 0.5f;
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_W)])
		{
			rigidBody->AddAcceleration(forward);
			//rigidBody->SetLinearVelocity();
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_S)])
		{
			rigidBody->AddAcceleration(-forward);
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_A)])
		{
			rigidBody->AddAcceleration(left);
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_D)])
		{
			rigidBody->AddAcceleration(-left);
		}
	}

	CameraComponent* GetCamera()
	{
		return m_Camera;
	}
};

class BaseAppContext : public AppContext
{
	SceneRendererDeferred* sceneRenderer;

	Actor* testActor = nullptr;

	~BaseAppContext() override
	{
		delete sceneRenderer;
	}

	// FIXME: this is just an hack!
	SceneRendererDeferred* GetSceneRenderer() override { return sceneRenderer; }

	void Init() override
	{
		SetGameContext<GameContext>();
		SwitchGameModeImmediately<GameModeBase>()->BeginPlay();

		sceneRenderer = new SceneRendererDeferred();

		AssimpModelLoader modelLoader;
		MeshImportedData importedData = modelLoader.ImportModel("Test", GEngine->GetResourceManager()->LoadFile("Assets/socuwan.fbx"));

		testActor = GetScene()->SpawnActor<Actor, StaticMeshComponent>("TestActor", Vector3(0, 0, 0), {}, Vector3(0.01f));

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

		MeshImportedData importedData2 = modelLoader.ImportModel("box", GEngine->GetResourceManager()->LoadFile("Assets/box_cubeUV.fbx"));
		auto matDef = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Textured.matd");
		auto matInstance = matDef->CreateInstance();
		matInstance->SetTexture("Texture"_HASH, GEngine->GetResourceManager()->LoadTexture("Assets/Textures/dry-rocky-ground-unity/dry-rocky-ground_albedo.png"));
		matInstance->SetTexture("NormalMap"_HASH, normalMap);

		if(true)
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
					Actor* groundActor = GetScene()->SpawnActor<Actor, StaticMeshComponent>("Ground " + std::to_string(x) + ", " + std::to_string(z), {x + 0.5f, -0.5f, z + 0.5f}, {}, Vector3(0.005f));
					groundActor->AddComponent<BoxColliderComponent>(1, 1, 1);

					if (x == 0 && z == 1)
					{
						groundActor->GetTransform().AddLocation(0, 1.0f, 0);
					}

					if(importedData2)
					{
						auto* meshComponent = StaticMeshComponent::Cast(groundActor->GetRootComponent());
						meshComponent->SetMesh(importedData2.Mesh);

						for (auto &item : meshComponent->GetMaterialSet())
						{
							item.second.Material = matInstance;
						}
					}
				}
			}
		}

		for (int i = 0; i < 10; ++i)
		{
			Actor* testActor2 = GetScene()->SpawnActor<Actor, StaticMeshComponent>("Box " + std::to_string(i), Vector3(i * 2.2f - 5, 0.5f, 0), {}, Vector3(0.005f));

			//RigidBodyComponent* rigidBodyComponent = testActor2->AddComponent<RigidBodyComponent>();
			//rigidBodyComponent->SetFriction(2.0f);

			BoxColliderComponent* collider = testActor2->AddComponent<BoxColliderComponent>(1, 1, 1);

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

		for (int i = 0; i < 10; ++i)
		{
			Actor* testActor2 = GetScene()->SpawnActor<Actor, StaticMeshComponent>("Box B " + std::to_string(i), Vector3(i * 2.2f - 5, 10 + i + 1, 0), {}, Vector3(0.005f));

			BoxColliderComponent* collider = testActor2->AddComponent<BoxColliderComponent>(1, 1, 1);

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

		Actor* camera = GetScene()->SpawnActor<CameraActor>("Camera", {0, 3, 5});

		GetScene()->SpawnActor<PointLight>("PointLight", {-1, 3, 1});

		if (!AppContext::IsEditorMode())
		{
			GEngine->GetInputManager()->LockCursor();
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