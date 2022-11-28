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

	inline float SRangeFloat(long seed, float min, float max)
	{
		std::mt19937 mt(seed);
		std::uniform_real_distribution<float> dist(min, max);
		return dist(mt);
	}

	inline int SRangeInt(long seed, int min, int max)
	{
		std::mt19937 mt(seed);
		std::uniform_int_distribution dist(min, max);
		return dist(mt);
	}

	inline int RangeInt(int min, int max)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution dist(min, max);
		return dist(mt);
	}
}