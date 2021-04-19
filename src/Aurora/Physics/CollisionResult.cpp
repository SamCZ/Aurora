#include "CollisionResult.hpp"

namespace Aurora
{
	struct compareCollisionResults
	{
		inline bool operator() (const CollisionResult& struct1, const CollisionResult& struct2)
		{
			return (struct1.Distance < struct2.Distance);
		}
	};

	CollisionResults::CollisionResults() : _results(), m_CurrentObject(nullptr)
	{
	}

	CollisionResults::~CollisionResults() = default;

	void CollisionResults::Clear()
	{
		_results.clear();
	}

	void CollisionResults::AddCollision(CollisionResult result)
	{
		result.Object = m_CurrentObject;
		_results.push_back(result);
		_sorted = false;
	}

	size_t CollisionResults::Size()
	{
		return _results.size();
	}

	CollisionResult CollisionResults::GetClosestCollision()
	{
		if (Size() == 0)
		{
			return CollisionResult();
		}
		if (!_sorted)
		{
			std::sort(_results.begin(), _results.end(), compareCollisionResults());
			_sorted = true;
		}
		return _results[0];
	}

	CollisionResult CollisionResults::GetFarthestCollision()
	{
		if (Size() == 0)
		{
			return CollisionResult();
		}
		if (!_sorted)
		{
			std::sort(_results.begin(), _results.end(), compareCollisionResults());
			_sorted = true;
		}
		return _results[Size() - 1];
	}

	CollisionResult CollisionResults::GetCollision(int index)
	{
		if (Size() == 0)
		{
			return CollisionResult();
		}
		if (!_sorted)
		{
			std::sort(_results.begin(), _results.end(), compareCollisionResults());
			_sorted = true;
		}
		return _results[index];
	}

	CollisionResult CollisionResults::GetCollisionDirect(int index)
	{
		if (Size() == 0)
		{
			return CollisionResult();
		}
		return _results[index];
	}

	void CollisionResults::SetCurrentObject(CollisionResultObject *actor)
	{
		m_CurrentObject = actor;
	}
}