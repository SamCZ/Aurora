#include "CollisionMatrix.hpp"
#include <iostream>

namespace Aurora
{
	std::vector<Layer::Hash_t> CollisionMatrix::m_CollisionMatrix(LayerEnum::NumLayers);

	void CollisionMatrix::SetCollision(const LayerEnum &who, const LayerEnum &target, bool can_collide)
	{
		if(can_collide) {
			m_CollisionMatrix[who] |= (1u << target);
		} else {
			m_CollisionMatrix[who] &= ~(1u << target);
		}
	}

	void CollisionMatrix::SetCollision(const String &who, const String &target, bool can_collide)
	{
		SetCollision(Layer::NameToLayer(who), Layer::NameToLayer(target), can_collide);
	}

	bool CollisionMatrix::CanCollide(const LayerEnum &who, const LayerEnum &target)
	{
		return m_CollisionMatrix[who] & (1u << target);
	}

	bool CollisionMatrix::CanCollide(const LayerEnum &who, const Layer &target)
	{
		return m_CollisionMatrix[who] & target.Hash();
	}

	bool CollisionMatrix::CanCollide(const Layer &who, const Layer &target)
	{
		for (int i = 0; i < LayerEnum::NumLayers; ++i) {
			if(who & (LayerEnum)i) {
				if(CanCollide((LayerEnum)i, target)) {
					return true;
				}
			}
		}
		return false;
	}
}