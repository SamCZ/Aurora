#pragma once

#include "LightComponent.hpp"
#include "Aurora/Core/Color.hpp"

namespace Aurora
{
	class PointLightComponent : public LightComponent
	{
	private:
		Color m_Color{};
		float m_Intensity{};
		float m_Radius{};
	public:
		PointLightComponent() = default;
		~PointLightComponent() override = default;

		inline void SetColor(const Color& color) noexcept { m_Color = color; }
		inline void SetIntensity(float intensity) noexcept { m_Intensity = intensity; }
		inline void SetRadius(float radius) noexcept { m_Radius = radius; }

		inline const Color& GetColor() const noexcept { return m_Color; }
		inline float GetIntensity() const noexcept { return m_Intensity; }
		inline float GetRadius() const noexcept { return m_Radius; }
	};
}