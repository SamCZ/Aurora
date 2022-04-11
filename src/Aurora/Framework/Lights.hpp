#pragma once

#include "Aurora/Graphics/Color.hpp"
#include "SceneComponent.hpp"
#include "Actor.hpp"

namespace Aurora
{
	class LightComponent : public SceneComponent
	{
	private:
		float m_Intensity;
		bool m_CastShadows;
		Vector3 m_LightColor;
	public:
		CLASS_OBJ(LightComponent, SceneComponent);

		LightComponent() : m_Intensity(1.0f), m_CastShadows(true), m_LightColor(1.0f) {}

		[[nodiscard]] float GetIntensity() const { return m_Intensity; }
		[[nodiscard]] float& GetIntensity() { return m_Intensity; }
		[[nodiscard]] Color GetColor() const { return m_LightColor; }
		[[nodiscard]] Vector3& GetColor() { return m_LightColor; }
	};

	class DirectionalLightComponent : public LightComponent
	{
	public:
		CLASS_OBJ(DirectionalLightComponent, LightComponent);
	};

	class PointLightComponent : public LightComponent
	{
	private:
		float m_Radius = 10.0f;
	public:
		CLASS_OBJ(PointLightComponent, LightComponent);

		[[nodiscard]] float GetRadius() const { return m_Radius; }
		[[nodiscard]] float& GetRadius() { return m_Radius; }
	};

	class SpotLightComponent : public LightComponent
	{
	public:
		CLASS_OBJ(SpotLightComponent, LightComponent);
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