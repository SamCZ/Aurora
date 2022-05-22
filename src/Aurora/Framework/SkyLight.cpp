#include "SkyLight.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Graphics/ViewPortManager.hpp"
#include "Aurora/Resource/ResourceManager.hpp"
#include "Aurora/Framework/Scene.hpp"
#include "Aurora/Framework/Lights.hpp"
#include "CameraComponent.hpp"

namespace Aurora
{
	void SkyLight::InitializeComponents()
	{
		SkyLightComponent* component = SkyLightComponent::Cast(GetRootComponent());
		component->SetMesh(GEngine->GetResourceManager()->LoadMesh("Assets/Shapes/Box.amesh"));

		/*auto cubemap = GEngine->GetRenderManager()->CreateCubeMap({
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Right.png",
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Left.png",
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Up.png",
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Down.png",
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Back.png",
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Front.png"
		}, true);*/

		Material_ptr material = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/AuroraSky.matd")->CreateInstance();
		material->RasterState(Pass::Ambient).CullMode = ECullMode::Back;
		material->RasterState(Pass::Depth).CullMode = ECullMode::Back;
		material->SetSortType(RenderSortType::Sky);

		material->SetTexture("_NoiseTex"_HASH, GEngine->GetResourceManager()->LoadTexture("Assets/Textures/soft-noise-256.jpg"));
		material->SetTexture("_NightBox"_HASH, GEngine->GetResourceManager()->LoadCubeMapDef("Assets/Textures/stars_box.cubemap"));

		material->BeforeMaterialBegin.Bind([](PassType_t pass, DrawCallState& state, CameraComponent* camera, Material* mat)
		{
			RenderViewPort* wp = camera->GetViewPort();

			Vector3 lightDir = Vector3(0, 1, 0);

			if (DirectionalLightComponent* dirLight = camera->GetOwner()->GetScene()->FindFirstComponent<DirectionalLightComponent>())
			{
				lightDir = dirLight->GetForwardVector();
			}

			state.Uniforms.SetVec2("u_ViewPort"_HASH, Vector2(wp->GetWidth(), wp->GetHeight()));
			state.Uniforms.SetMat4x4("u_InvProjection"_HASH, glm::inverse(camera->GetProjectionMatrix()));
			state.Uniforms.SetMat4x4("u_InvView"_HASH, glm::inverse(camera->GetViewMatrix()));
			state.Uniforms.SetVec3("u_ViewPos"_HASH, camera->GetWorldPosition());
			state.Uniforms.SetVec3("u_LightDir"_HASH, lightDir);
			state.Uniforms.SetFloat("u_Time"_HASH, 0);
		});

		component->SetMaterial(0, material);
	}
}