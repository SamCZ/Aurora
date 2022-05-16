#pragma once

#include "Aurora/Core/Library.hpp"
#include "Aurora/Core/Vector.hpp"
#include "Aurora/Framework/Physics/ColliderComponent.hpp"
#include "AABBTree.hpp"

namespace Aurora
{
	class Scene;

	struct RayCastHitResult
	{
		class Actor* HitActor;
		Vector3 HitPosition;
		Vector3 HitNormal;
		double HitDistance;
	};

	class AU_API PhysicsWorld
	{
	private:
		Scene* m_Scene;
		double m_Time;
		double m_Accumulator;

		bool m_DebugRender;

		Vector3 m_Gravity;
		double m_UpdateRate;

		AABBTree<ColliderComponent> m_AABBTree;
	public:
		explicit PhysicsWorld(Scene* scene);
		~PhysicsWorld();

		inline void SetDebugRender(bool debugRender) { m_DebugRender = debugRender; }
		[[nodiscard]] inline bool IsDebugRender() const { return m_DebugRender; }
		inline void ToggleDebugRender() { m_DebugRender = !m_DebugRender; }

		void Update(double frameTime);

		int32_t RayCast(const Vector3& fromPos, const Vector3& toPos, std::vector<RayCastHitResult>& results) const;
	private:
		void RunPhysics();
	};
}