#include "Random.hpp"

namespace Aurora
{
	Random::Random(unsigned int seed) : _Seed(seed)
	{

	}

	Random::Random() : _Seed(std::random_device()())
	{

	}

	int Random::GetInt(int min, int max)
	{
		_RandomGenerator.seed(_Seed++);

		std::uniform_int_distribution<std::mt19937::result_type> dist6(min, max);

		return dist6(_RandomGenerator);
	}

	float Random::GetFloat(float min, float max)
	{
		_RandomGenerator.seed(_Seed++);

		std::uniform_real_distribution<float> dist6(min, max);

		return dist6(_RandomGenerator);
	}

	double Random::GetDouble(double min, double max)
	{
		_RandomGenerator.seed(_Seed++);

		std::uniform_real_distribution<double> dist6(min, max);

		return dist6(_RandomGenerator);
	}

	bool Random::GetByPercent(float percent)
	{
		return GetFloat(0, 100) <= percent;
	}

	Vector3 Random::GetRandomUnitVector3()
	{
		return glm::normalize(Vector3(GetFloat(-1.0f, 1.0f), GetFloat(-1.0f, 1.0f), GetFloat(-1.0f, 1.0f)));
	}

	Vector2 Random::GetRandomUnitVector2()
	{
		return Vector2(GetFloat(-1.0f, 1.0f), GetFloat(-1.0f, 1.0f));
	}

	Vector3D Random::GetRandomUnitVector3D()
	{
		return glm::normalize(Vector3D(GetDouble(-1.0f, 1.0f), GetDouble(-1.0f, 1.0f), GetDouble(-1.0f, 1.0f)));
	}

	Vector2D Random::GetRandomUnitVector2D()
	{
		return glm::normalize(Vector2D(GetDouble(-1.0f, 1.0f), GetDouble(-1.0f, 1.0f)));
	}
}