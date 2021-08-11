#include "PhysicsBody.hpp"
#include "Aurora/AuroraEngine.hpp"

namespace Aurora
{

	PhysicsBody::~PhysicsBody()
	{
		RemoveBody();
	}

	void PhysicsBody::RemoveBody()
	{
		if(m_Body)
		{
			AuroraEngine::Physics->DeleteBody(m_Body);
			m_Body = nullptr;
		}
	}

	void PhysicsBody::SetBody(ndBodyKinematic *newBody, bool addToWorld)
	{
		if(m_Body == newBody) return;
		RemoveBody();

		m_Body = newBody;

		if(addToWorld)
		{
			AuroraEngine::Physics->AddBody(newBody);
		}
	}
}
