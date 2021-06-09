#pragma once

#include <vector>
#include <Aurora/Core/Vector.hpp>

namespace Aurora
{
	class Actor;

	typedef Actor CollisionResultObject;

	struct CollisionResult
	{
		CollisionResultObject* Object = nullptr;
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
		CollisionResultObject* m_CurrentObject;
	public:
		CollisionResults();
		~CollisionResults();
		void Clear();
		void AddCollision(CollisionResult result);
		size_t Size();

		void SetCurrentObject(CollisionResultObject* actor);

		CollisionResult GetClosestCollision();
		CollisionResult GetFarthestCollision();
		CollisionResult GetCollision(int index);
		CollisionResult GetCollisionDirect(int index);
	};
}