#include "SkyLight.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/RenderManager.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

namespace Aurora
{
	void SkyLight::InitializeComponents()
	{
		SkyLightComponent* component = SkyLightComponent::Cast(GetRootComponent());
		component->SetMesh(GEngine->GetResourceManager()->LoadMesh("Assets/Shapes/Box.amesh"));

		auto cubemap = GEngine->GetRenderManager()->CreateCubeMap({
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Right.png",
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Left.png",
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Up.png",
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Down.png",
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Back.png",
			"Assets/Textures/CloudyCrown_01_Midday/CloudyCrown_Midday_Front.png"
		}, true);

		Material_ptr material = GEngine->GetResourceManager()->GetOrLoadMaterialDefinition("Assets/Materials/Base/SkyBox.matd")->CreateInstance();
		material->RasterState(Pass::Ambient).CullMode = ECullMode::Back;
		material->RasterState(Pass::Depth).CullMode = ECullMode::Back;
		material->SetSortType(RenderSortType::Sky);
		material->SetMacro("SKY_MAPPING", "1");

		material->SetTexture("CubeMap"_HASH, cubemap);

		component->SetMaterial(0, material);
	}
}