#pragma once

#include <cstdint>
#include <sstream>
#include <glm/glm.hpp>

namespace Aurora
{
	struct Color
	{
		union
		{
			struct
			{
				uint8_t r{};
				uint8_t g{};
				uint8_t b{};
				uint8_t a{};
			};
			uint32_t rgba;
		};

		Color() {} // This is intention so we don't initialize data automatically

		constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
				: r(r), g(g), b(b), a(a) {}

		constexpr Color(uint32_t rgba)
				: rgba(rgba) {}

		Color(const glm::vec3& rgb) : r(glm::round(rgb.x * 255.0f)), g(glm::round(rgb.y * 255.0f)), b(glm::round(rgb.z * 255.0f)), a(1.0f) { }
		Color(const glm::vec4& rgb) : r(glm::round(rgb.x * 255.0f)), g(glm::round(rgb.y * 255.0f)), b(glm::round(rgb.z * 255.0f)), a(glm::round(rgb.w * 255.0f)) { }

		static constexpr Color red() { return {255, 0, 0}; }
		static constexpr Color green() { return {0, 255, 0}; }
		static constexpr Color blue() { return {0, 0, 255}; }
		static constexpr Color black() { return {0, 0, 0}; }
		static constexpr Color white() { return {255, 255, 255}; }
		static constexpr Color zero() { return {0, 0, 0, 0}; }

		operator glm::vec3() const
		{
			return {r / 255.0f, g / 255.0f, b / 255.0f};
		}

		operator glm::vec4() const
		{
			return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
		}
	};

	static std::ostream& operator<<(std::ostream& os, const Color& color)
	{
		os << (int)color.r << "," << (int)color.g << "," << (int)color.b << "," << (int)color.a;
		return os;
	}
}