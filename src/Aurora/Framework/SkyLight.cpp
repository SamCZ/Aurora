#include "SkyLight.hpp"

#include "Aurora/Engine.hpp"
#include "Aurora/Resource/ResourceManager.hpp"

namespace Aurora
{
	void SkyLight::InitializeComponents()
	{
		SkyLightComponent* component = SkyLightComponent::Cast(GetRootComponent());

		// TODO: Load cube mesh
		//component->SetMesh();
	}
}