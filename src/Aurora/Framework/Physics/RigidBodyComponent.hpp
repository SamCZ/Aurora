#pragma once

#include "Aurora/Framework/Actor.hpp"
#include "Aurora/Framework/SceneComponent.hpp"
#include <btBulletDynamicsCommon.h>

namespace Aurora
{
	class AU_API RigidBodyComponent : public SceneComponent, public btMotionState
	{
	private:
		btRigidBody* m_Body = nullptr;
		btCompoundShape* m_CompoundShape = nullptr;
	public:
		CLASS_OBJ(RigidBodyComponent, SceneComponent);

		RigidBodyComponent();
		~RigidBodyComponent() override;

		void getWorldTransform(btTransform& out_transform) const override
		{
			out_transform.setFromOpenGLMatrix(glm::value_ptr(GetOwner()->GetTransform().GetTransformNoScale()));
		}

		void setWorldTransform(const btTransform& transform) override
		{
			Matrix4 mat;
			transform.getOpenGLMatrix(glm::value_ptr(mat));
			GetOwner()->GetTransform().SetFromMatrixNoScale(mat);
		}

		void SetBody(btRigidBody* body);
		void AddShape(btCollisionShape* shape);
		void RemoveShape(btCollisionShape* shape);

		inline btRigidBody* GetBody() const { return m_Body; }
		inline Vector3 GetLocalInertia() const { return btToVec(m_Body->getLocalInertia()); }

		inline void SetMass(float mass)
		{
			btCollisionShape* shape = m_Body->getCollisionShape();
			btVector3 localInertia(0, 0, 0);

			if (mass != 0.0)
			{
				shape->calculateLocalInertia(mass, localInertia);
			}

			m_Body->setMassProps(mass, localInertia);
		}

		inline float GetMass() const { return m_Body->getMass(); }
	};
}
