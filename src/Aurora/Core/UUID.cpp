#include "UUID.hpp"

#include <random>

namespace Aurora
{

	UUID UUID::Generate()
	{
		std::random_device rd;
		std::mt19937_64 e2(rd());
		std::uniform_int_distribution<uint64_t> distribution;
		return UUID(distribution(e2), distribution(e2));
	}
}