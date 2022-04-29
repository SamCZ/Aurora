#include <Aurora/Aurora.hpp>

#include <Aurora/Resource/ResourceManager.hpp>

#include <Aurora/Graphics/Material/MaterialDefinition.hpp>
#include <Aurora/Graphics/ViewPortManager.hpp>

#include <Shaders/World/PBRBasic/cb_pbr.h>

#include <Aurora/Memory/Aum.hpp>
#include <Aurora/Framework/Actor.hpp>
#include <Aurora/Framework/SceneComponent.hpp>
#include <Aurora/Framework/CameraComponent.hpp>
#include <Aurora/Framework/StaticMeshComponent.hpp>
#include <Aurora/Framework/Lights.hpp>
#include <Aurora/Framework/Physics/RigidBodyComponent.hpp>
#include <Aurora/Physics/CollisionMatrix.hpp>

#include <Aurora/Resource/AssimpModelLoader.hpp>

#include "Aurora/Editor/MainEditorPanel.hpp"

#include <Aurora/Graphics/VgRender.hpp>
#include <Aurora/Graphics/DShape.hpp>

#include <Aurora/Render/SceneRenderer.hpp>
#include <Aurora/Physics/BulletDebugDraw.hpp>

#include <btBulletDynamicsCommon.h>

#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

using namespace Aurora;

class CameraActor : public Actor
{
private:
	CameraComponent* m_Camera;

	btRigidBody* m_Body;
public:
	CLASS_OBJ(CameraActor, Actor);
	DEFAULT_COMPONENT(CameraComponent);

	void InitializeComponents() override
	{
		m_Camera = CameraComponent::Cast(GetRootComponent());
		m_Camera->SetName("Camera");
		m_Camera->SetViewPort(GEngine->GetViewPortManager()->Get());
		m_Camera->SetPerspective(75, 0.1f, 2000.0f);

		RigidBodyComponent* bodyComponent = AddComponent<RigidBodyComponent>("Physics");
		this->CollisionLayer = Layer::NameToLayer("Player");

		btConvexInternalShape* shape = new btSphereShape(0.5f);

		btScalar mass(1);
		btVector3 localInertia(0, 0, 0);
		shape->calculateLocalInertia(mass, localInertia);

		bodyComponent->setWorldTransform(btFromMatrix(glm::translate(Vector3(0, 10, -2))));
		btRigidBody::btRigidBodyConstructionInfo cInfo(mass, bodyComponent, shape, localInertia);
		m_Body = new btRigidBody(cInfo);

		m_Body->setAngularFactor(0);
		m_Body->setActivationState(DISABLE_DEACTIVATION);

		bodyComponent->SetBody(m_Body);
	}

	void Tick(double delta) override
	{
		//GetTransform().SetFromMatrixNoScale(btToMatrix(m_Controller->getGhostObject()->getWorldTransform()));

		float m_FlySpeed = 10.0f;

		float yaw = ImGui::GetIO().MouseDelta.y * -1.0f;
		float pitch = ImGui::GetIO().MouseDelta.x * -1.0f;
		//m_Camera->GetTransform().AddRotation(yaw, pitch, 0.0f);
		m_Body->setAngularVelocity(btVector3(yaw, pitch, 0));

		Matrix4 transform = m_Camera->GetTransformationMatrix();

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_W)])
		{
			//m_Camera->GetTransform().AddLocation(-Vector3(transform[2]) * (float)delta * m_FlySpeed);
			m_Body->translate(btFromVec3(-Vector3(transform[2]) * (float)delta * m_FlySpeed));
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_S)])
		{
			//m_Camera->GetTransform().AddLocation(Vector3(transform[2]) * (float)delta * m_FlySpeed);
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_A)])
		{
			//m_Camera->GetTransform().AddLocation(-Vector3(transform[0]) * (float)delta * m_FlySpeed);
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_D)])
		{
			//m_Camera->GetTransform().AddLocation(Vector3(transform[0]) * (float)delta * m_FlySpeed);
		}

		if(ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_Space)])
		{
			m_Body->applyCentralImpulse(btVector3(0.0f, 0.1f, 0.0f));

			//m_Camera->GetTransform().AddLocation(Vector3(transform[0]) * (float)delta * m_FlySpeed);
		}
	}

	CameraComponent* GetCamera()
	{
		return m_Camera;
	}
};

class BaseAppContext : public AppContext
{
	SceneRenderer* sceneRenderer;
	CameraActor* cameraActor;
	Actor* testActor = nullptr;
	Actor* box = nullptr;

	~BaseAppContext() override
	{
		delete sceneRenderer;
	}

	// FIXME: this is just an hack!
	SceneRenderer* GetSceneRenderer() override { return sceneRenderer; }

	void Init() override
	{
		SetGameContext<GameContext>();
		SwitchGameModeImmediately<GameModeBase>()->BeginPlay();

		sceneRenderer = new SceneRenderer();

		// m_dynamicsWorld->setSynchronizeAllMotionStates(true);

		Layer::Setup({
			"Ground",
			"Box",
			"Player"
		});
		//CollisionMatrix::SetCollision("Box", "Ground", true);
		CollisionMatrix::SetCollision("Ground", "Box", true);
		CollisionMatrix::SetCollision("Ground", "Player", true);

		{ // Create floor shape
			RigidBodyComponent* bodyComponent = GetScene()->SpawnActor<Actor, RigidBodyComponent>("Floor", {0, -50, 0})->GetRootComponent<RigidBodyComponent>();
			bodyComponent->GetOwner()->CollisionLayer = Layer("Ground");

			auto* groundShape = new btBoxShape(btVector3(btScalar(50), btScalar(50), btScalar(50)));

			btScalar mass(0.);
			btVector3 localInertia(0, 0, 0);
			btRigidBody::btRigidBodyConstructionInfo cInfo(mass, bodyComponent, groundShape, localInertia);
			btRigidBody* body = new btRigidBody(cInfo);
			bodyComponent->SetBody(body);
		}

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

		MeshImportOptions meshImportOptions;
		meshImportOptions.DefaultScale = 0.01f;
		MeshImportedData importedData2 = modelLoader.ImportModel("box", GEngine->GetResourceManager()->LoadFile("Assets/box_cubeUV.fbx"), meshImportOptions);
		auto matDef = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Textured.matd");
		auto matInstance = matDef->CreateInstance();
		matInstance->SetTexture("Texture"_HASH, GEngine->GetResourceManager()->LoadTexture("Assets/Textures/dry-rocky-ground-unity/dry-rocky-ground_albedo.png"));
		matInstance->SetTexture("NormalMap"_HASH, normalMap);

		for (int i = 0; i < 10; ++i)
		{
			Actor* testActor2 = GetScene()->SpawnActor<Actor, StaticMeshComponent>("Box", Vector3(i * 2.2f, 0, -20), {}, Vector3(1.0f));

			if (true)
			{
				testActor2->GetTransform().SetLocation(i * 2.2f - 6, 10, 0);
				testActor2->CollisionLayer = Layer("Box");
				RigidBodyComponent* rigidBodyComponent = testActor2->AddComponent<RigidBodyComponent>("RigidBody");

				btConvexInternalShape* shape;

				if (i == 4)
				{
					shape = new btSphereShape(1.5f);
				}
				else
				{
					shape = new btBoxShape(btVector3(btScalar(1), btScalar(1), btScalar(1)));
				}

				btScalar mass(1);
				btVector3 localInertia(0, 0, 0);
				shape->calculateLocalInertia(mass, localInertia);

				btRigidBody::btRigidBodyConstructionInfo cInfo(mass, rigidBodyComponent, shape, localInertia);
				btRigidBody* body = new btRigidBody(cInfo);
				body->setFriction(5);
				rigidBodyComponent->SetBody(body);
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

		cameraActor = GetScene()->SpawnActor<CameraActor>("Camera", {0, 0, 5});

		GetScene()->SpawnActor<PointLight>("PointLight", {-1, 3, 1});

		if (!AppContext::IsEditorMode())
		{
			GEngine->GetInputManager()->LockCursor();
		}
	}

	class RayClosestResult : btCollisionWorld::ClosestRayResultCallback
	{
		Layer CollisionLayer;

		RayClosestResult(const btVector3& rayFromWorld, const btVector3& rayToWorld) : btCollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld) {}
		RayClosestResult(const Ray& ray, float distance, Layer collisionLayer) : btCollisionWorld::ClosestRayResultCallback({}, {}), CollisionLayer(collisionLayer)
		{
			m_rayFromWorld = btVector3(ray.Origin.x, ray.Origin.y, ray.Origin.z);
			m_rayToWorld = m_rayFromWorld + btVector3(ray.Direction.x, ray.Direction.y, ray.Direction.z) * distance;
		}

		bool needsCollision(btBroadphaseProxy* proxy0) const override
		{
			btCollisionObject* obj0 = static_cast<btCollisionObject*>(proxy0->m_clientObject);
			Actor* actor0 = static_cast<Actor*>(obj0->getUserPointer());

			return CollisionMatrix::CanCollide(CollisionLayer, actor0->CollisionLayer);
		}
	};

	void Update(double delta) override
	{
		static bool lastKeyState = false;

		if(lastKeyState != ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_F)])
		{
			lastKeyState = ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_F)];

			if (lastKeyState)
			{
				Ray ray = cameraActor->GetCamera()->GetRayFromScreen((Vector2i)cameraActor->GetCamera()->GetViewPort()->ViewPort / 2);

				btVector3 rayFromWorld(ray.Origin.x, ray.Origin.y, ray.Origin.z);
				btVector3 rayToWorld = rayFromWorld + btVector3(ray.Direction.x, ray.Direction.y, ray.Direction.z) * 10.0f;

				btCollisionWorld::ClosestRayResultCallback rayCallback(rayFromWorld, rayToWorld);
				GetScene()->GetDynamicsWorld()->rayTest(rayFromWorld, rayToWorld, rayCallback);

				if (rayCallback.hasHit())
				{
					btVector3 pickPos = rayCallback.m_hitPointWorld;
					btRigidBody* body = (btRigidBody*)btRigidBody::upcast(rayCallback.m_collisionObject);
					if (body)
					{
						btVector3 localPivot = body->getCenterOfMassTransform().inverse() * pickPos;
						body->applyImpulse(-rayCallback.m_hitNormalWorld * 2.0f, localPivot);
						body->activate();
					}
				}
			}
		}

		/*for (int j = m_dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
		{
			btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[j];
			btRigidBody* body = btRigidBody::upcast(obj);
			btTransform trans;
			if (body && body->getMotionState())
			{
				body->getMotionState()->getWorldTransform(trans);
			}
			else
			{
				trans = obj->getWorldTransform();
			}

			if (j == 1)
			{
				box->GetTransform().Location.x = trans.getOrigin().getX();
				box->GetTransform().Location.y = trans.getOrigin().getY();
				box->GetTransform().Location.z = trans.getOrigin().getZ();

				Vector3 rot;
				trans.getRotation().getEulerZYX(rot.x, rot.y, rot.z);
				box->GetTransform().Rotation = glm::degrees(rot);

				//printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
			}

			//printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));

		}*/
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
	windowDefinition.Title = "Aurora - Physics";

	Aurora::AuroraEngine engine;
	engine.Init(new BaseAppContext(), windowDefinition, true);
	engine.Run();
	return 0;
}