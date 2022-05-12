#include "RigidBodyComponent.hpp"
#include "../SceneComponent.hpp"

namespace Aurora
{
	void RigidBodyComponent::PredictIntegratedTransform(phScalar timeStep, Aurora::Transform& predictedTransform)
	{
		phTransformUtil::IntegrateTransform(GetWorldTransform(), m_LinearVelocity, m_AngularVelocity, timeStep, predictedTransform);
	}

	Transform& RigidBodyComponent::GetWorldTransform()
	{
		return GetParent()->GetTransform();
	}
}