#pragma once

#include <random>

namespace Aurora::Rand
{
	inline float RandFloat()
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<float> dist;
		return dist(mt);
	}

	inline float RangeFloat(float min, float max)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<float> dist(min, max);
		return dist(mt);
	}
}