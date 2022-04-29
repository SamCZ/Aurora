#include "RigidBodyComponent.hpp"

namespace Aurora
{
	RigidBodyComponent::RigidBodyComponent() :
		SceneComponent(),
		btMotionState(),
		m_Body(nullptr),
		m_CompoundShape(new btCompoundShape())
	{

	}

	RigidBodyComponent::~RigidBodyComponent()
	{
		if (m_Body != nullptr)
		{
			delete m_Body->getCollisionShape();
		}
		delete m_Body;
	}

	void RigidBodyComponent::SetBody(btRigidBody *body)
	{
		if (m_Body)
		{
			m_Scene->GetDynamicsWorld()->removeRigidBody(m_Body);
			delete m_Body;
		}

		m_Body = body;
		m_Body->setUserPointer(GetOwner());
		m_Scene->GetDynamicsWorld()->addRigidBody(m_Body);
	}

	void RigidBodyComponent::AddShape(btCollisionShape* shape)
	{
		
	}

	void RigidBodyComponent::RemoveShape(btCollisionShape* shape)
	{

	}
}