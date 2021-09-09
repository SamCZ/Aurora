#pragma once

#include "LightComponent.hpp"
#include "Aurora/Core/Color.hpp"

namespace Aurora
{
	class PointLightComponent : public LightComponent
	{
	private:
		Vector3 m_Radiance = { 1.0f, 1.0f, 1.0f };
		float m_Intensity = 1.0f;
		float m_LightSize = 0.5f; // For PCSS
		float m_MinRadius = 1.f;
		float m_Radius = 10.f;
		bool m_CastsShadows = true;
		bool m_SoftShadows = true;
		float m_Falloff = 1.f;
	public:
		PointLightComponent() = default;
		~PointLightComponent() override = default;

		inline void SetRadiance(const Color& color) noexcept { m_Radiance = color; }
		inline void SetIntensity(float intensity) noexcept { m_Intensity = intensity; }
		inline void SetRadius(float radius) noexcept { m_Radius = radius; }

		inline const Vector3& GetRadiance() const noexcept { return m_Radiance; }
		inline float GetIntensity() const noexcept { return m_Intensity; }
		inline float GetRadius() const noexcept { return m_Radius; }
	};
}