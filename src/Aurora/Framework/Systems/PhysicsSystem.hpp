#pragma once

#include "../ISystem.hpp"
#include "../Components/Base/SceneComponent.hpp"
#include "../Actor.hpp"
#include <Aurora/Physics/CollisionMatrix.hpp>

namespace Aurora
{
	class SubCollisionSystem
	{
	private:
		Layer m_Layer;
	public:
		SubCollisionSystem() : m_Layer() { }
		explicit SubCollisionSystem(const Layer& layer) : m_Layer(layer) { }
		virtual ~SubCollisionSystem() = default;
	public:
		virtual void Collide(class Actor* actor, SceneComponent* rootComponent, PhysicsBody* body, Vector3D& position, Vector3D& velocity, Vector3D& acceleration, double delta) = 0;
	public:
		Layer& GetLayer() { return m_Layer; }
	};

	class PhysicsSystem : public ISystem<SceneComponent>
	{
	private:
		double m_TerminalVelocity;
		std::vector<SubCollisionSystem*> m_SubCollisionSystems;
	public:
		PhysicsSystem();
		~PhysicsSystem() override;

		void Update(const std::vector<SceneComponent*>& components, double delta) override;

		void ApplyTerminalVelocity(Vector3D& vec) const;

	public:
		template<class T, typename... Args>
		void AddSubSystem(Args... args)
		{
			m_SubCollisionSystems.push_back(new T(std::forward<T>(args...)));
		}
	};
}