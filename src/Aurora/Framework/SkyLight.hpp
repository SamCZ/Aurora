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

	class SkyLightComponent : public StaticMeshComponent
	{
	private:
		SkyLightMode m_Mode;
		Vector3 m_Color;
	public:
		CLASS_OBJ(SkyLightComponent, StaticMeshComponent);

		SkyLightComponent() : m_Mode(SkyLightMode::SkyBox), m_Color(0.7f) {}

		[[nodiscard]] inline SkyLightMode GetMode() const { return m_Mode; }
		inline void SetMode(SkyLightMode mode) { m_Mode = mode; }
	};

	class SkyLight : public Actor
	{
	public:
		CLASS_OBJ(SkyLight, Actor);
		DEFAULT_COMPONENT(SkyLightComponent);

		void InitializeComponents() override;
	};
}
