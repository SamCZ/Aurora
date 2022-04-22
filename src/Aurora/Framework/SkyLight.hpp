#pragma once

#include "Actor.hpp"
#include "StaticMeshComponent.hpp"
#include "Aurora/Graphics/Material/Material.hpp"

namespace Aurora
{
	enum class SkyLightMode : uint8_t
	{
		Color, Gradient, AuroraSky, SkyBox, Custom
	};

	static const char* SkyLightMode_Strings[] = { "Color", "Gradient", "AuroraSky", "SkyBox", "Custom" };

	class AU_API SkyLightComponent : public StaticMeshComponent
	{
	private:
		SkyLightMode m_Mode;
		float m_AmbientIntensity;
		Vector3 m_Color;
		Vector3 m_AmbientColor;
	public:
		CLASS_OBJ(SkyLightComponent, StaticMeshComponent);

		SkyLightComponent() : m_Mode(SkyLightMode::SkyBox), m_AmbientIntensity(0.0f), m_Color(0.7f), m_AmbientColor(1.0f) {}

		[[nodiscard]] inline SkyLightMode GetMode() const { return m_Mode; }
		inline void SetMode(SkyLightMode mode) { m_Mode = mode; }

		void SetAmbientIntensity(float intensity) { m_AmbientIntensity = intensity; }
		float GetAmbientIntensity() const { return m_AmbientIntensity; }

		void SetAmbientColor(const Vector3& color) { m_AmbientColor = color; }
		const Vector3& GetAmbientColor() const { return m_AmbientColor; }
	};

	class AU_API SkyLight : public Actor
	{
	public:
		CLASS_OBJ(SkyLight, Actor);
		DEFAULT_COMPONENT(SkyLightComponent);

		void InitializeComponents() override;
	};
}
