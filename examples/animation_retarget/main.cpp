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
#include <Aurora/Framework/SkeletalMeshComponent.hpp>
#include <Aurora/Framework/Lights.hpp>
#include <Aurora/Framework/ParticleSystemComponent.hpp>

#include <Aurora/Resource/AssimpModelLoader.hpp>
#include <Aurora/Resource/ResourceName.hpp>

#include "Aurora/Editor/MainEditorPanel.hpp"

#include <Aurora/Graphics/VgRender.hpp>

#include <Aurora/Render/SceneRendererDeferred.hpp>
#include <Aurora/Render/SceneRendererForward.hpp>
#include <Aurora/Render/SceneRendererDeferredNew.hpp>

#include <Aurora/Framework/AnimationProcessing/FbxImporter.hpp>

using namespace Aurora;

class BaseAppContext : public AppContext
{
	SceneRenderer* sceneRenderer;

	~BaseAppContext() override
	{
		delete sceneRenderer;

	}

	// FIXME: this is just an hack!
	SceneRenderer* GetSceneRenderer() override { return sceneRenderer; }

	StaticMesh_ptr mesh;
	StaticMesh_ptr finalMesh;
	Actor* planeActor;

	void Init() override
	{
		SetGameContext<GameContext>();
		SwitchGameModeImmediately<GameModeBase>()->BeginPlay();

		sceneRenderer = new SceneRendererForward();

		{ // Setup camera
			CameraComponent* camera = AppContext::GetScene()->SpawnActor<Actor, CameraComponent>("Camera", {0, 0, -5})->GetRootComponent<CameraComponent>();
			camera->SetViewPort(GEngine->GetViewPortManager()->Get(0));
			camera->SetPerspective(90.0f, 0.1, 200.0f);
		}

		//FbxImport::LoadScene(GEngine->GetResourceManager()->LoadFile("Assets/pickaxe.fbx"));

		MeshImportOptions importOptions;
		importOptions.SplitMeshes = false;
		importOptions.KeepCPUData = true;
		importOptions.UploadToGPU = true;

		AssimpModelLoader modelLoader;
		MeshImportedData importedData = modelLoader.ImportModel("mesh", GEngine->GetResourceManager()->LoadFile("Assets/sphere.fbx"), importOptions);

		mesh = importedData.Get<StaticMesh>();

		//mesh = StaticMesh::Cast(GEngine->GetResourceManager()->LoadMesh("Assets/Shapes/Sphere.amesh"));

		{ // Mesh obj
			Material_ptr material = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Color.matd");

			StaticMeshComponent* meshComponent = AppContext::GetScene()->SpawnActor<Actor, StaticMeshComponent>("Mesh")->GetRootComponent<StaticMeshComponent>();
			meshComponent->SetMesh(mesh);

			for (int i = 0; i < meshComponent->GetNumMaterialSlots(); ++i)
			{
				meshComponent->SetMaterial(i, material);
			}

			meshComponent->GetTransform().SetScale(0.01f);
			meshComponent->GetTransform().SetLocation(0, 0.5f, 0);
		}

		{ // Mesh obj final
			FMeshSection section;

			finalMesh = MakeShared<StaticMesh>();
			finalMesh->CreateVertexBuffer<StaticMesh::Vertex>(0);
			finalMesh->LODResources[0].Sections.push_back(section);
			finalMesh->MaterialSlots[0] = MaterialSlot(nullptr, "Main");

			Material_ptr material = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Color.matd");
			StaticMeshComponent* meshComponent = AppContext::GetScene()->SpawnActor<Actor, StaticMeshComponent>("Mesh")->GetRootComponent<StaticMeshComponent>();
			meshComponent->SetMesh(finalMesh);

			for (int i = 0; i < meshComponent->GetNumMaterialSlots(); ++i)
			{
				meshComponent->SetMaterial(i, material);
			}

			meshComponent->GetTransform().SetLocation(3, 0, 0);
		}

		planeActor = AppContext::GetScene()->SpawnActor<Actor>("Plane");
		planeActor->GetTransform().SetLocation(0, 0.025, 0);
		planeActor->GetTransform().SetRotation(90, 0, 0);
	}

	float DistFromPlane(const vec4& plane, const vec3& point)
	{
		return glm::dot(vec3(plane), point) + plane.w;
	}

	bool IsOnPlaneSide(const vec4& plane, const vec3& point)
	{
		float d1 = DistFromPlane(plane, point);
		return d1 < glm::epsilon<float>();
	}

	bool IsOnPlane(const vec4& plane, const vec3& point)
	{
		float d1 = DistFromPlane(plane, point);
		return abs(d1) >= glm::epsilon<float>();
	}

	int ClipPoly(const Vector3 *pts, int npts, Vector3 *dest, const Vector4& plane)
	{
		int sides[32];
		float dists[32];
		int numsides[2];

		numsides[0] = numsides[1] = 0;

		//qualify sides
		for (int v = 0; v < npts; v++)
		{
			vec3 point = pts[v];
			float dist = glm::dot(vec3(plane), point) + plane.w;
			int side = (dist >= 0) ? 0 : 1;
			dists[v] = dist;
			sides[v] = side;
			numsides[side]++;
		}

		if (numsides[1] == npts)
			return 0;

		if (numsides[0] == npts)
		{
			return -1;
		}

		//clip poly
		int nvert = 0;

		for (int v0 = 0; v0 < npts && nvert < 31; v0++)
		{
			int v1 = (v0 + 1) % npts;

			if (sides[v0] == 0) //inside->copy
			{
				dest[nvert++] = pts[v0];
			}

			if (sides[v1] == sides[v0]) //both on the same side
				continue;

			float dot = dists[v0] / (dists[v0] - dists[v1]);
			dest[nvert++] = glm::mix(pts[v0], pts[v1], dot);
		}

		return nvert;
	}

	void Update(double delta) override
	{
		VertexBuffer<StaticMesh::Vertex>* vertexBuffer = mesh->GetVertexBuffer<StaticMesh::Vertex>();
		VertexBuffer<StaticMesh::Vertex>* finalVertexBuffer = finalMesh->GetVertexBuffer<StaticMesh::Vertex>();
		finalVertexBuffer->Clear();
		finalMesh->LODResources[0].Indices.clear();

		//vec4 plane(0, 1, 0, 0.25);
		std::vector<vec4> clipPlanes(1);
		clipPlanes[0] = vec4(planeActor->GetRootComponent()->GetForwardVector(), glm::distance(planeActor->GetRootComponent()->GetWorldPosition(), vec3(0, -0.5f, 0)));

		int finalIndex = 0;

		for (const auto& [lod, lodData]: mesh->LODResources)
		{
			for (int i = 0; i < lodData.Indices.size(); i += 3)
			{
				uint i0 = lodData.Indices[i + 0];
				uint i1 = lodData.Indices[i + 1];
				uint i2 = lodData.Indices[i + 2];

				vec3 v0 = vertexBuffer->Get(i0).Position * 0.01f;
				vec3 v1 = vertexBuffer->Get(i1).Position * 0.01f;
				vec3 v2 = vertexBuffer->Get(i2).Position * 0.01f;

				v0.y += 0.5f;
				v1.y += 0.5f;
				v2.y += 0.5f;

				Vector3 pts[3] = {
					v0, v1, v2
				};

				Vector3* srcpoints = pts;

				int npoints = 3;

				static const int maxPoints = 16;
				Vector3 clippoints[2][maxPoints];

				Vector3* destpoints = clippoints[0];
				uint	flip = 0;

				for(uint p = 0; p < clipPlanes.size(); p++)
				{
					int res = ClipPoly(srcpoints, npoints, destpoints, clipPlanes[p]);
					if(res == 0)
						continue;
					if(res < 0)
						continue;

					npoints = res;
					srcpoints = clippoints[flip];
					flip ^= 1;
					destpoints = clippoints[flip];
				}

				uint index = 1;

				//build triangle list indices
				for(uint n = 0; n < npoints - 2; n++)
				{
					vec3 cv0 = srcpoints[0];
					vec3 cv1 = srcpoints[index];
					index++;
					vec3 cv2 = srcpoints[index % npoints];

					if (IsOnPlaneSide(clipPlanes[0], cv0) && IsOnPlaneSide(clipPlanes[0], cv1) && IsOnPlaneSide(clipPlanes[0], cv2))
					{

					}
					else
					{
						if (IsOnPlane(clipPlanes[0], cv0) && IsOnPlane(clipPlanes[0], cv1) && IsOnPlane(clipPlanes[0], cv2))
						{
							DShapes::WireTriangle(cv0, cv1, cv2, Color::green());
						}
						else
						{
							DShapes::WireTriangle(cv0, cv1, cv2, Color::blue());
						}

						finalVertexBuffer->Add(StaticMesh::Vertex(cv0));
						finalVertexBuffer->Add(StaticMesh::Vertex(cv1));
						finalVertexBuffer->Add(StaticMesh::Vertex(cv2));

						finalMesh->LODResources[0].Indices.push_back(finalIndex++);
						finalMesh->LODResources[0].Indices.push_back(finalIndex++);
						finalMesh->LODResources[0].Indices.push_back(finalIndex++);
					}
				}

				//DShapes::WireTriangle(v0, v1, v2, Color::blue());

			}
		}

		finalMesh->LODResources[0].Sections[0].NumTriangles = finalIndex;

		finalMesh->UploadToGPU(true, true);
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