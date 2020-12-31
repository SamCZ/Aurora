#pragma once

#include <random>
#include "Vector.hpp"

namespace Aurora
{
	class Random
	{
	private:
		unsigned int _Seed;
		std::mt19937 _RandomGenerator;
	public:
		Random(unsigned int seed);
		Random();

		int GetInt(int min, int max);
		float GetFloat(float min, float max);
		double GetDouble(double min, double max);
		bool GetByPercent(float percent);

		Vector3 GetRandomUnitVector3();
		Vector2 GetRandomUnitVector2();

		Vector3D GetRandomUnitVector3D();
		Vector2D GetRandomUnitVector2D();
	};
}