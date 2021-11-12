#pragma once

#include <vector>
#include "Aurora/Framework/Layer.hpp"

namespace Aurora
{
	class CollisionMatrix
	{
	private:
		static std::vector<Layer::Hash_t> m_CollisionMatrix;
	public:
		static void SetCollision(const LayerEnum& who, const LayerEnum& target, bool can_collide);
		static void SetCollision(const String& who, const String& target, bool can_collide);
		static bool CanCollide(const LayerEnum& who, const LayerEnum& target);
		static bool CanCollide(const LayerEnum& who, const Layer& target);
		static bool CanCollide(const Layer& who, const Layer& target);
	};
}
