#pragma once

#include "Aurora/Graphics/Color.hpp"
#include "SceneComponent.hpp"
#include "Actor.hpp"

namespace Aurora
{
	class LightComponentBase : public SceneComponent
	{
	private:
		float m_Intensity;
		bool m_CastShadows;
		Color m_Color;
	public:
		CLASS_OBJ(LightComponentBase, SceneComponent);
	};

	class DirectionalLightComponent : public LightComponentBase
	{
	public:
		CLASS_OBJ(DirectionalLightComponent, LightComponentBase);
	};

	class PointLightComponent : public LightComponentBase
	{
	public:
		CLASS_OBJ(PointLightComponent, LightComponentBase);
	};

	class SpotLightComponent : public LightComponentBase
	{
	public:
		CLASS_OBJ(SpotLightComponent, LightComponentBase);
	};

	///////////////////////////////////////////////////////////////////////////////////

	class LightBase : public Actor
	{
	public:
		CLASS_OBJ(LightBase, Actor);
	};

	class DirectionalLight : public LightBase
	{
	public:
		CLASS_OBJ(DirectionalLight, LightBase);
		DEFAULT_COMPONENT(DirectionalLightComponent);
	};

	class PointLight : public LightBase
	{
	public:
		CLASS_OBJ(PointLight, LightBase);
		DEFAULT_COMPONENT(PointLightComponent);
	};

	class SpotLight : public LightBase
	{
	public:
		CLASS_OBJ(SpotLight, LightBase);
		DEFAULT_COMPONENT(SpotLightComponent);
	};
}