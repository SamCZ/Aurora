#pragma once

#include "Vector.hpp"

namespace Aurora
{
	struct ColorRGBA
	{
		static const ColorRGBA White;
		static const ColorRGBA Black;

		static const ColorRGBA Red;
		static const ColorRGBA Green;
		static const ColorRGBA Blue;

		float r, g, b, a;

        ColorRGBA() : r(0), g(0), b(0), a(0)
        {

        }

        ColorRGBA(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a)
        {

        }

		[[nodiscard]] inline Vector4 toVec4() const
		{
			return Vector4(r, g, b, a);
		}
		[[nodiscard]] inline Vector3 toVec3() const
		{
			return Vector3(r, g, b);
		}

		inline ColorRGBA operator+=(const ColorRGBA& other)
		{
			r += other.r;
			g += other.g;
			b += other.b;
			a += other.a;
			return *this;
		}

		inline ColorRGBA operator+(const ColorRGBA& other)
		{
			r += other.r;
			g += other.g;
			b += other.b;
			a += other.a;
			return *this;
		}

        bool operator==( const ColorRGBA& Other ) const
        {
            return r == Other.r && g == Other.g && b == Other.b && a == Other.a;
        }
        bool operator!=( const ColorRGBA& Other ) const
        {
            return !(*this == Other);
        }

	};

	ColorRGBA MakeRGB(int r, int g, int b);
	ColorRGBA MakeRGBA(int r, int g, int b, int a);
	ColorRGBA MakeRGBf(float r, float g, float b);
	ColorRGBA MakeRGBAf(float r, float g, float b, float a);
	ColorRGBA MakeHEX(const char* hex);
}