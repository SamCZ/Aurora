#pragma once

#include <Aurora/Core/Vector.hpp>

namespace Aurora
{
	struct CollisionResult
	{
		void* Object = nullptr;
		Vector3D ContactPoint = {};
		Vector3D ContactNormal = {};
		double Distance = 0;
		int TriangleIndex = 0;

		CollisionResult() = default;
	};

	class CollisionResults
	{
	private:
		std::vector<CollisionResult> _results;
		bool _sorted = false;
	public:
		CollisionResults();
		~CollisionResults();
		void Clear();
		void AddCollision(const CollisionResult& result);
		int Size();

		CollisionResult GetClosestCollision();
		CollisionResult GetFarthestCollision();
		CollisionResult GetCollision(int index);
		CollisionResult GetCollisionDirect(int index);
	};
}