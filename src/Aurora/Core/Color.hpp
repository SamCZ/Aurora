#pragma once

#include <sstream>
#include "Vector.hpp"

namespace Aurora
{
	struct CompactColor
	{
		union
		{
			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
				uint8_t a;
			};
			uint32_t rgba;
		};
	};

	inline Vector4 MakeRGB(int r, int g, int b)
	{
		Vector4 color;
		color.x = (float)r / 255.0f;
		color.y = (float)g / 255.0f;
		color.z = (float)b / 255.0f;
		color.w = 1.0f;
		return color;
	}

	inline Vector4 MakeRGBA(int r, int g, int b, int a)
	{
		Vector4 color;
		color.x = (float)r / 255.0f;
		color.y = (float)g / 255.0f;
		color.z = (float)b / 255.0f;
		color.w = (float)a / 255.0f;
		return color;
	}

	inline Vector4 MakeRGBf(float r, float g, float b)
	{
		Vector4 color;
		color.x = r;
		color.y = g;
		color.z = b;
		color.w = 1.0f;
		return color;
	}

	inline Vector4 MakeRGBAf(float r, float g, float b, float a)
	{
		Vector4 color;
		color.x = r;
		color.y = g;
		color.z = b;
		color.w = a;
		return color;
	}

	inline Vector4 MakeHEX(const char* hex)
	{
		size_t len = strlen(hex);
		if (len == 6)
		{
			std::stringstream ss;
			ss << std::hex << hex[0] << std::hex << hex[1];
			int r;
			ss >> r;
			ss.clear();
			ss << std::hex << hex[2] << std::hex << hex[3];
			int g;
			ss >> g;
			ss.clear();
			ss << std::hex << hex[4] << std::hex << hex[5];
			int b;
			ss >> b;
			ss.clear();

			return MakeRGB(r, g, b);
		}
		return Vector4(1.0f);
	}

	typedef glm::vec4 Color;

	namespace Colors
	{
		static const Vector4 White = MakeRGB(255, 255, 255);
		static const Vector4 Black = MakeRGB(0, 0, 0);
		static const Vector4 Red = MakeRGB(255, 0, 0);
		static const Vector4 Green = MakeRGB(0, 255, 0);
		static const Vector4 Blue = MakeRGB(0, 0, 255);
		static const Vector4 Transparent = Vector4(0, 0, 0, 0);
	}
}