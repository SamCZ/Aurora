#include "ColorRGBA.hpp"

#include <iostream>
#include <sstream>

namespace Aurora
{
	ColorRGBA MakeRGB(int r, int g, int b)
	{
		ColorRGBA color;
		color.r = (float)r / 255.0f;
		color.g = (float)g / 255.0f;
		color.b = (float)b / 255.0f;
		color.a = 1.0f;
		return color;
	}

	ColorRGBA MakeRGBA(int r, int g, int b, int a)
	{
		ColorRGBA color;
		color.r = (float)r / 255.0f;
		color.g = (float)g / 255.0f;
		color.b = (float)b / 255.0f;
		color.a = (float)a / 255.0f;
		return color;
	}

	ColorRGBA MakeRGBf(float r, float g, float b)
	{
		ColorRGBA color;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = 1.0f;
		return color;
	}

	ColorRGBA MakeRGBAf(float r, float g, float b, float a)
	{
		ColorRGBA color;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;
		return color;
	}

	ColorRGBA MakeHEX(const char* hex)
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
		return MakeRGB(255, 255, 255);
	}

	const ColorRGBA ColorRGBA::White = MakeRGB(255, 255, 255);
	const ColorRGBA ColorRGBA::Black = MakeRGB(0, 0, 0);

	const ColorRGBA ColorRGBA::Red = MakeRGB(255, 0, 0);
	const ColorRGBA ColorRGBA::Green = MakeRGB(0, 255, 0);
	const ColorRGBA ColorRGBA::Blue = MakeRGB(0, 0, 255);
}