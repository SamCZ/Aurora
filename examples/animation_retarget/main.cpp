#include <Aurora/Aurora.hpp>

#include <Aurora/Core/AUID.hpp>
#include <Aurora/Core/Random.hpp>
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

#include <Aurora/Tools/ImGuiHelper.hpp>

using namespace Aurora;

struct Triangle
{
	union {
		struct
		{
			vec3 V0;
			vec3 V1;
			vec3 V2;
		};

		vec3 Points[3];
	};

	Triangle() {}

	Triangle(const vec3& v0, const vec3& v1, const vec3& v2)
		: V0(v0), V1(v1), V2(v2) {}
};

vec3 GetTriNormal(const Triangle& tri)
{
	return glm::normalize(glm::cross(tri.V0 - tri.V2, tri.V0 - tri.V1));
}

enum class TriangleClipResult : uint8_t
{
	InFront,
	Back,
	Clipped
};

static TriangleClipResult ClipTriangle(const Triangle& inputTriangle, const Vector4& plane, std::vector<Triangle>& outTriangles)
{
	constexpr uint8_t PointsInTriangle = 3;

	int sides[PointsInTriangle];
	float distances[PointsInTriangle];
	int numSides[2];
	numSides[0] = numSides[1] = 0;

	vec3 planeNormal = vec3(plane);

	for (uint8_t i = 0; i < PointsInTriangle; ++i)
	{
		float dist = glm::dot(planeNormal, inputTriangle.Points[i]) + plane.w;
		uint8_t side = (dist >= 0) ? 0u : 1u;

		sides[i] = side;
		distances[i] = dist;
		numSides[side]++;
	}

	if (numSides[1] == PointsInTriangle)
		return TriangleClipResult::InFront;

	if (numSides[0] == PointsInTriangle)
		return TriangleClipResult::Back;

	vec3 output[16];
	int npoints = 0;
	for (uint8_t v0 = 0; v0 < PointsInTriangle && npoints < 16; v0++)
	{
		int v1 = (v0 + 1) % PointsInTriangle;

		if (sides[v0] == 0) //inside->copy
		{
			output[npoints++] = inputTriangle.Points[v0];
		}

		if (sides[v1] == sides[v0]) //both on the same side
		{
			continue;
		}

		float dot = distances[v0] / (distances[v0] - distances[v1]);
		output[npoints++] = glm::mix(inputTriangle.Points[v0], inputTriangle.Points[v1], dot);
	}

	uint index = 1;

	//build triangle list indices
	for(uint n = 0; n < npoints - 2; n++)
	{
		vec3 cv0 = output[0];
		vec3 cv1 = output[index];
		index++;
		vec3 cv2 = output[index % npoints];

		outTriangles.emplace_back(cv0, cv1, cv2);
	}

	return TriangleClipResult::Clipped;
}

bool IsPointOnPlane(const vec3& point, const vec4& plane)
{
	float dist = glm::dot(vec3(plane), point) + plane.w;

	if (glm::abs(dist) <= 0.00001f)
	{
		return true;
	}

	return false;
}

bool IsPointNearlySame(const vec3& left, const vec3& right)
{
	float d = glm::distance(left, right);
	return d <= 0.00001f;
}

bool FindPointOnPlane(const vec4& plane, const Triangle& tri, const vec3* ignorePoint, vec3& foundPoint)
{
	bool hasIgnorePoint = false;
	bool foundAnotherPointOnPlane = false;

	for (int i = 0; i < 3; i++)
	{
		const vec3& point = tri.Points[i];

		if (ignorePoint != nullptr && IsPointNearlySame(point, *ignorePoint))
		{
			hasIgnorePoint = true;
			continue;
		}

		if (IsPointOnPlane(point, plane))
		{
			foundPoint = point;
			foundAnotherPointOnPlane = true;

			if (hasIgnorePoint)
				break;
		}
	}

	return (ignorePoint == nullptr || hasIgnorePoint) && foundAnotherPointOnPlane;
}

bool IsPointInTriangle(const vec3& point, const Triangle& tri)
{
	for (int i = 0; i < 3; ++i)
	{
		if (IsPointNearlySame(point, tri.Points[i]))
			return true;
	}
	return false;
}


std::vector<Triangle> clippedTrisTest;


void FillClippedHole(const vec4& plane, std::vector<Triangle> clipped, std::vector<Triangle>& leftTriangles, std::vector<Triangle>& rightTriangle)
{
	if (clipped.empty())
		return;

	// Find path
	std::vector<vec3> pathPoints;

	//clippedTrisTest = clipped;
	Triangle firstTriangle = clipped[0];


	// Find first point on plane
	vec3 foundFistPoint;
	if (not FindPointOnPlane(plane, clipped[0], nullptr, foundFistPoint))
	{
		AU_LOG_FATAL("Fist point on plane not found !");
		return;
	}

	pathPoints.emplace_back(foundFistPoint);
	clipped.erase(clipped.begin());

	uint32_t numIterations = 0;

	// Find path along triangles
	while (true)
	{
		if (numIterations > 200'000)
		{
			AU_LOG_INFO("Something went wrong! Triangle path finding was stuck in infinite loop");
			break;
		}

		const vec3& startPoint = pathPoints[pathPoints.size() - 1];

		if (pathPoints.size() > 2 && IsPointInTriangle(startPoint, firstTriangle))
			break;

		vec3 newPathPoint;
		bool nextPointFound = false;

		for (int i = 0; i < clipped.size(); ++i)
		{
			if (FindPointOnPlane(plane, clipped[i], &startPoint, newPathPoint))
			{
				pathPoints.push_back(newPathPoint);
				nextPointFound = true;
				clipped.erase(clipped.begin() + i);
				break;
			}
		}

		if (not nextPointFound)
		{
			AU_LOG_INFO("Triangle path did not found next triangle");
			break;
		}

		numIterations++;
	}

	// Old style - find middle point and build triangles around that
	std::vector<vec3> pointsInsidePlane;
	vec3 mid = vec3(0.0f);

	// Find points on the plane and remove duplicates
	for (const vec3& point: pathPoints)
	{
		//for (int i = 0; i < 3; ++i)
		{
			//const vec3& point = tri.Points[i];

			//if (IsPointOnPlane(point, plane))
			{
				bool found = false;
				for (const vec3& p: pointsInsidePlane)
				{
					if (glm::distance(p, point) < 0.00001f)
					{
						found = true;
					}
				}

				if (not found)
				{
					pointsInsidePlane.push_back(point);
					mid += point;
				}
			}
		}
	}

	// Build new triangles
	// TODO: improve filling be knowing the path of points do not create triangles intersecting with path

	int32_t numPoints = int32_t(pointsInsidePlane.size());

	mid /= float(numPoints);

	for (int v0 = 0; v0 < numPoints; ++v0)
	{
		int v1 = (v0 + 1) % numPoints;
		//leftTriangles.emplace_back(pointsInsidePlane[v0], mid, pointsInsidePlane[v1]);
		//rightTriangle.emplace_back(pointsInsidePlane[v0], pointsInsidePlane[v1], mid);

		clippedTrisTest.emplace_back(pointsInsidePlane[v0], mid, pointsInsidePlane[v1]);
	}
}

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
	StaticMeshComponent* meshComponent;

	std::vector<Triangle> OriginalMeshTriangles;
	std::vector<std::vector<Triangle>> SplitPieces;

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

		planeActor = AppContext::GetScene()->SpawnActor<Actor>("Plane");
		planeActor->GetTransform().SetLocation(0, 0.025, 0);
		planeActor->GetTransform().SetRotation(90, 0, 0);

		//FbxImport::LoadScene(GEngine->GetResourceManager()->LoadFile("Assets/pickaxe.fbx"));

		MeshImportOptions importOptions;
		importOptions.SplitMeshes = false;
		importOptions.KeepCPUData = true;
		importOptions.UploadToGPU = true;

		AssimpModelLoader modelLoader;
		MeshImportedData importedData = modelLoader.ImportModel("mesh", GEngine->GetResourceManager()->LoadFile("Assets/weird_shape.fbx"), importOptions);

		mesh = importedData.Get<StaticMesh>();

		//mesh = StaticMesh::Cast(GEngine->GetResourceManager()->LoadMesh("Assets/Shapes/Sphere.amesh"));

		{ // Mesh obj
			if (true)
			{
				Material_ptr material = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/Color.matd");

				meshComponent = AppContext::GetScene()->SpawnActor<Actor, StaticMeshComponent>("Mesh")->GetRootComponent<StaticMeshComponent>();
				meshComponent->SetMesh(mesh);

				for (int i = 0; i < meshComponent->GetNumMaterialSlots(); ++i)
				{
					meshComponent->SetMaterial(i, material);
				}

				meshComponent->GetTransform().SetScale(0.01f);
				meshComponent->GetTransform().SetLocation(0, 0.5f, 0);
			}

			OriginalMeshTriangles.clear();

			VertexBuffer<StaticMesh::Vertex>* vertexBuffer = mesh->GetVertexBuffer<StaticMesh::Vertex>();
			//VertexBuffer<StaticMesh::Vertex>* finalVertexBuffer = finalMesh->GetVertexBuffer<StaticMesh::Vertex>();
			//finalVertexBuffer->Clear();
			//finalMesh->LODResources[0].Indices.clear();

			//vec4 plane(0, 1, 0, 0.25);
			std::vector<vec4> clipPlanes;
			clipPlanes.push_back(vec4(planeActor->GetRootComponent()->GetForwardVector(), glm::distance(planeActor->GetRootComponent()->GetWorldPosition(), vec3(0, -0.5f, 0))));

			for (int i = 0; i < 3; ++i)
			{
				//clipPlanes.push_back(vec4(Rand::RangeFloat(-1, 1), Rand::RangeFloat(-1, 1), Rand::RangeFloat(-1, 1), Rand::RangeFloat(-0.5, 0.5)));
				int a = 0;
			}

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

					OriginalMeshTriangles.emplace_back(v0, v1, v2);
				}
			}

			SplitPieces.push_back(OriginalMeshTriangles);
			bool removedFirst = false;

			for (const vec4& plane: clipPlanes)
			{
				std::vector<std::vector<Triangle>> splitPiecesCopy = SplitPieces;
				SplitPieces.clear();

				for (const std::vector<Triangle>& pieceTriangles : splitPiecesCopy)
				{
					std::vector<Triangle> splitLeft;
					std::vector<Triangle> splitLeftClipped;
					std::vector<Triangle> splitRight;

					for (const Triangle& tri : pieceTriangles)
					{
						if (ClipTriangle(tri, plane, splitLeftClipped) == TriangleClipResult::Back)
						{
							splitLeft.push_back(tri);
						}

						if (ClipTriangle(tri, -plane, splitRight) == TriangleClipResult::Back)
						{
							splitRight.push_back(tri);
						}
					}

					splitLeft.insert(splitLeft.begin(), splitLeftClipped.begin(), splitLeftClipped.end());

					FillClippedHole(plane, splitLeftClipped, splitLeft, splitRight);

					if (not splitLeft.empty())
						SplitPieces.emplace_back(splitLeft);

					if (not splitRight.empty())
						SplitPieces.emplace_back(splitRight);
				}
			}
		}

		if (false)
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
	}

	typedef struct HsvColor
	{
		unsigned char h;
		unsigned char s;
		unsigned char v;
	} HsvColor;

	Color HsvToRgb(HsvColor hsv)
	{
		Color rgb;
		unsigned char region, remainder, p, q, t;

		if (hsv.s == 0)
		{
			rgb.r = hsv.v;
			rgb.g = hsv.v;
			rgb.b = hsv.v;
			return rgb;
		}

		region = hsv.h / 43;
		remainder = (hsv.h - (region * 43)) * 6;

		p = (hsv.v * (255 - hsv.s)) >> 8;
		q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
		t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

		switch (region)
		{
			case 0:
				rgb.r = hsv.v; rgb.g = t; rgb.b = p;
				break;
			case 1:
				rgb.r = q; rgb.g = hsv.v; rgb.b = p;
				break;
			case 2:
				rgb.r = p; rgb.g = hsv.v; rgb.b = t;
				break;
			case 3:
				rgb.r = p; rgb.g = q; rgb.b = hsv.v;
				break;
			case 4:
				rgb.r = t; rgb.g = p; rgb.b = hsv.v;
				break;
			default:
				rgb.r = hsv.v; rgb.g = p; rgb.b = q;
				break;
		}

		return rgb;
	}

	void Update(double delta) override
	{
		VertexBuffer<StaticMesh::Vertex>* vertexBuffer = mesh->GetVertexBuffer<StaticMesh::Vertex>();
		//VertexBuffer<StaticMesh::Vertex>* finalVertexBuffer = finalMesh->GetVertexBuffer<StaticMesh::Vertex>();
		//finalVertexBuffer->Clear();
		//finalMesh->LODResources[0].Indices.clear();

		//vec4 plane(0, 1, 0, 0.25);

		static float distanceMod = 1.0f;
		ImGui::SliderFloat("Explode", &distanceMod, 0, 1);

		if (distanceMod < 0.0001)
		{
			meshComponent->GetOwner()->SetActive(true);
		}
		else
		{
			meshComponent->GetOwner()->SetActive(false);

			int i = 0;
			int inc = 255 / SplitPieces.size();
			for (const std::vector<Triangle>& pieceTriangles : SplitPieces)
			{
				vec3 off = vec3(0.0f);

				HsvColor hsvColor = {(unsigned char)(i * inc), 255, 255};
				Color c = HsvToRgb(hsvColor);

				vec3 mid = vec3(0.0f);
				for (const Triangle& tri : pieceTriangles)
					for (int j = 0; j < 3; ++j)
						mid += tri.Points[j];
				mid /= float(pieceTriangles.size() * 3);

				vec3 normalFromCenter = glm::normalize(mid - vec3(0.0f));
				float distTo0 = glm::distance(mid, vec3(0.0f));

				//off = normalFromCenter * distTo0 * distanceMod;

				for (const Triangle& tri : pieceTriangles)
				{
					vec3 norm = GetTriNormal(tri);
					vec3 normColor = norm * 0.5f + 0.5f;

					DShapes::Triangle(tri.V2 + off, tri.V1 + off, tri.V0 + off, Color(normColor));
				}
				break;
				i++;
			}
		}

		for (const auto& tri: clippedTrisTest)
		{
			vec3 norm = GetTriNormal(tri);
			vec3 normColor = norm * 0.5f + 0.5f;

			DShapes::WireTriangle(tri.V2, tri.V1, tri.V0, Color::red());
		}

		//for (const auto& item: pathPoints)
		//{
		//	DShapes::WireBox(AABB(item - 0.01f, item + 0.01f));
		//}



		//finalMesh->LODResources[0].Sections[0].NumTriangles = finalIndex;

		//finalMesh->UploadToGPU(true, true);
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